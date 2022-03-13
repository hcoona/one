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

#include "one/jinduo/net/tcp_client.h"

#include <stdio.h>  // snprintf

#include <utility>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/internal/connector.h"
#include "one/jinduo/net/internal/sockets_ops.h"

namespace jinduo {
namespace net {

namespace details {

void removeConnection(EventLoop* loop,
                      const std::shared_ptr<TcpConnection>& conn) {
  loop->queueInLoop(absl::bind_front(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const std::shared_ptr<Connector>& /*connector*/) {
  // connector->
}

}  // namespace details

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr,
                     std::string nameArg)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, serverAddr)),
      name_(std::move(nameArg)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1) {
  connector_->setNewConnectionCallback(
      absl::bind_front(&TcpClient::newConnection, this));
  // FIXME setConnectFailedCallback
  LOG(INFO) << "TcpClient::TcpClient[" << name_ << "] - connector "
            << connector_.get();
}

TcpClient::~TcpClient() {
  LOG(INFO) << "TcpClient::~TcpClient[" << name_ << "] - connector "
            << connector_.get();
  std::shared_ptr<TcpConnection> conn;
  bool unique = false;
  {
    absl::MutexLock lock(&mutex_);
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn) {
    assert(loop_ == conn->getLoop());
    // FIXME: not 100% safe, if we are in different thread
    CloseCallback cb = absl::bind_front(&details::removeConnection, loop_);
    loop_->runInLoop(
        absl::bind_front(&TcpConnection::setCloseCallback, conn, cb));
    if (unique) {
      conn->forceClose();
    }
  } else {
    connector_->stop();
    // FIXME: HACK
    loop_->runAfter(absl::Seconds(1),
                    absl::bind_front(&details::removeConnector, connector_));
  }
}

void TcpClient::connect() {
  // FIXME: check state
  LOG(INFO) << "TcpClient::connect[" << name_ << "] - connecting to "
            << connector_->serverAddress().toIpPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;

  {
    absl::MutexLock lock(&mutex_);
    if (connection_) {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();
  InetAddress peerAddr(sockets::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  std::shared_ptr<TcpConnection> conn(
      new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));

  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      absl::bind_front(&TcpClient::removeConnection, this));  // FIXME: unsafe
  {
    absl::MutexLock lock(&mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const std::shared_ptr<TcpConnection>& conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    absl::MutexLock lock(&mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(absl::bind_front(&TcpConnection::connectDestroyed, conn));
  if (retry_ && connect_) {
    LOG(INFO) << "TcpClient::connect[" << name_ << "] - Reconnecting to "
              << connector_->serverAddress().toIpPort();
    connector_->restart();
  }
}

}  // namespace net
}  // namespace jinduo
