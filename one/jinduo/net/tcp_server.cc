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

#include "one/jinduo/net/tcp_server.h"

#include <stdio.h>  // snprintf

#include <atomic>
#include <utility>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/event_loop_thread_pool.h"
#include "one/jinduo/net/internal/acceptor.h"
#include "one/jinduo/net/internal/sockets_ops.h"

namespace jinduo {
namespace net {

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                     std::string nameArg, Option option)
    : loop_(CHECK_NOTNULL(loop)),
      ipPort_(listenAddr.toIpPort()),
      name_(std::move(nameArg)),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback) {
  acceptor_->SetNewConnectionCallback(
      absl::bind_front(&TcpServer::newConnection, this));
}

TcpServer::~TcpServer() {
  loop_->AssertInLoopThread();
  VLOG(1) << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto& item : connections_) {
    std::shared_ptr<TcpConnection> conn(item.second);
    item.second.reset();
    conn->getLoop()->RunInLoop(
        absl::bind_front(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::set_thread_num(int numThreads) {
  assert(0 <= numThreads);
  threadPool_->set_thread_num(numThreads);
}

void TcpServer::start() {
  bool expected = false;
  if (started_.compare_exchange_strong(expected, true,
                                       std::memory_order_acq_rel)) {
    threadPool_->Start(threadInitCallback_);

    assert(!acceptor_->listening());
    loop_->RunInLoop(absl::bind_front(&Acceptor::Listen, acceptor_.get()));
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
  loop_->AssertInLoopThread();
  EventLoop* ioLoop = threadPool_->GetNextLoop();
  char buf[64];
  snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
  ++nextConnId_;
  std::string connName = name_ + buf;

  VLOG(1) << "TcpServer::newConnection [" << name_ << "] - new connection ["
          << connName << "] from " << peerAddr.toIpPort();
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  std::shared_ptr<TcpConnection> conn(
      new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(
      absl::bind_front(&TcpServer::removeConnection, this));  // FIXME: unsafe
  ioLoop->RunInLoop(absl::bind_front(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const std::shared_ptr<TcpConnection>& conn) {
  // FIXME: unsafe
  loop_->RunInLoop(
      absl::bind_front(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(
    const std::shared_ptr<TcpConnection>& conn) {
  loop_->AssertInLoopThread();
  VLOG(1) << "TcpServer::removeConnectionInLoop [" << name_ << "] - connection "
          << conn->name();
  size_t n = connections_.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->QueueInLoop(absl::bind_front(&TcpConnection::connectDestroyed, conn));
}

}  // namespace net
}  // namespace jinduo
