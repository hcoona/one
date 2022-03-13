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
//
// This is a public header file, it must only include public header files.

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "one/jinduo/net/tcp_connection.h"

namespace jinduo {
namespace net {

class Connector;

class TcpClient {
 public:
  // TcpClient(EventLoop* loop);
  // TcpClient(EventLoop* loop, const std::string& host, uint16_t port);
  TcpClient(EventLoop* loop, const InetAddress& serverAddr,
            std::string nameArg);
  ~TcpClient();  // force out-line dtor, for std::unique_ptr members.

  // Disallow copy.
  TcpClient(const TcpClient&) noexcept = delete;
  TcpClient& operator=(const TcpClient&) noexcept = delete;

  // Allow move but not implemented yet.
  TcpClient(TcpClient&&) noexcept = delete;
  TcpClient& operator=(TcpClient&&) noexcept = delete;

  void connect();
  void disconnect();
  void stop();

  std::shared_ptr<TcpConnection> connection() const {
    absl::MutexLock lock(&mutex_);
    return connection_;
  }

  EventLoop* getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }

  const std::string& name() const { return name_; }

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(ConnectionCallback cb) {
    connectionCallback_ = std::move(cb);
  }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(MessageCallback cb) {
    messageCallback_ = std::move(cb);
  }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(WriteCompleteCallback cb) {
    writeCompleteCallback_ = std::move(cb);
  }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd);
  /// Not thread safe, but in loop
  void removeConnection(const std::shared_ptr<TcpConnection>& conn);

  EventLoop* loop_;
  std::shared_ptr<Connector> connector_;  // avoid revealing Connector
  const std::string name_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  bool retry_;    // atomic
  bool connect_;  // atomic
  // always in loop thread
  int nextConnId_;
  mutable absl::Mutex mutex_;
  std::shared_ptr<TcpConnection> connection_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace jinduo
