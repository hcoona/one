// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.
//
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/net/internal/connector.h"

#include <errno.h>

#include <algorithm>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "memory"
#include "one/jinduo/base/strerror.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/internal/channel.h"
#include "one/jinduo/net/internal/sockets_ops.h"

namespace jinduo {
namespace net {

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop* loop, const InetAddress& serverAddr)
    : loop_(loop),
      serverAddr_(serverAddr),
      connect_(false),
      state_(kDisconnected),
      retryDelayMs_(kInitRetryDelayMs) {
  VLOG(1) << "ctor[" << this << "]";
}

Connector::~Connector() {
  VLOG(1) << "dtor[" << this << "]";
  assert(!channel_);
}

void Connector::start() {
  connect_ = true;
  loop_->RunInLoop(
      absl::bind_front(&Connector::startInLoop, this));  // FIXME: unsafe
}

void Connector::startInLoop() {
  loop_->AssertInLoopThread();
  assert(state_ == kDisconnected);
  if (connect_) {
    connect();
  } else {
    VLOG(1) << "do not connect";
  }
}

void Connector::stop() {
  connect_ = false;
  loop_->QueueInLoop(
      absl::bind_front(&Connector::stopInLoop, this));  // FIXME: unsafe
  // FIXME: cancel timer
}

void Connector::stopInLoop() {
  loop_->AssertInLoopThread();
  if (state_ == kConnecting) {
    setState(kDisconnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}

void Connector::connect() {
  int sockfd = sockets::createNonblockingOrDie(serverAddr_.family());
  int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch (savedErrno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);
      break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG(ERROR) << "connect error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      break;

    default:
      LOG(ERROR) << "Unexpected error in Connector::startInLoop " << savedErrno;
      sockets::close(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::restart() {
  loop_->AssertInLoopThread();
  setState(kDisconnected);
  retryDelayMs_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

void Connector::connecting(int sockfd) {
  setState(kConnecting);
  assert(!channel_);
  channel_ = std::make_unique<Channel>(loop_, sockfd);
  channel_->setWriteCallback(
      absl::bind_front(&Connector::handleWrite, this));  // FIXME: unsafe
  channel_->setErrorCallback(
      absl::bind_front(&Connector::handleError, this));  // FIXME: unsafe

  // channel_->tie(shared_from_this()); is not working,
  // as channel_ is not managed by shared_ptr
  channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  // Can't reset channel_ here, because we are inside Channel::handleEvent
  loop_->QueueInLoop(
      absl::bind_front(&Connector::resetChannel, this));  // FIXME: unsafe
  return sockfd;
}

void Connector::resetChannel() { channel_.reset(); }

void Connector::handleWrite() {
  VLOG(1) << "Connector::handleWrite " << state_;

  if (state_ == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    if (err != 0) {
      LOG(WARNING) << "Connector::handleWrite - SO_ERROR = " << err << " "
                   << strerror_tl(err);
      retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
      LOG(WARNING) << "Connector::handleWrite - Self connect";
      retry(sockfd);
    } else {
      setState(kConnected);
      if (connect_) {
        newConnectionCallback_(sockfd);
      } else {
        sockets::close(sockfd);
      }
    }
  } else {
    // what happened?
    assert(state_ == kDisconnected);
  }
}

void Connector::handleError() {
  LOG(ERROR) << "Connector::handleError state=" << state_;
  if (state_ == kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    VLOG(1) << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sockfd);
  }
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(kDisconnected);
  if (connect_) {
    LOG(INFO) << "Connector::retry - Retry connecting to "
              << serverAddr_.toIpPort() << " in " << retryDelayMs_
              << " milliseconds. ";
    loop_->RunAfter(
        absl::Milliseconds(retryDelayMs_),
        absl::bind_front(&Connector::startInLoop, shared_from_this()));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  } else {
    VLOG(1) << "do not connect";
  }
}

}  // namespace net
}  // namespace jinduo
