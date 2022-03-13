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

#include <atomic>
#include <map>
#include <memory>
#include <string>

#include "one/jinduo/net/tcp_connection.h"

namespace jinduo {
namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class TcpServer {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  enum Option {
    kNoReusePort,
    kReusePort,
  };

  // TcpServer(EventLoop* loop, const InetAddress& listenAddr);
  TcpServer(EventLoop* loop, const InetAddress& listenAddr, std::string nameArg,
            Option option = kNoReusePort);
  ~TcpServer();  // force out-line dtor, for std::unique_ptr members.

  // Disallow copy.
  TcpServer(const TcpServer&) noexcept = delete;
  TcpServer& operator=(const TcpServer&) noexcept = delete;

  // Allow move but not implemented yet.
  TcpServer(TcpServer&&) noexcept = delete;
  TcpServer& operator=(TcpServer&&) noexcept = delete;

  [[nodiscard]] const std::string& ipPort() const { return ipPort_; }
  [[nodiscard]] const std::string& name() const { return name_; }
  [[nodiscard]] EventLoop* getLoop() const { return loop_; }

  /// Set the number of threads for handling input.
  ///
  /// Always accepts new connection in loop's thread.
  /// Must be called before @c start
  /// @param numThreads
  /// - 0 means all I/O in loop's thread, no thread will created.
  ///   this is the default value.
  /// - 1 means all I/O in another thread.
  /// - N means a thread pool with N threads, new connections
  ///   are assigned on a round-robin basis.
  void setThreadNum(int numThreads);
  void setThreadInitCallback(const ThreadInitCallback& cb) {
    threadInitCallback_ = cb;
  }
  /// valid after calling start()
  std::shared_ptr<EventLoopThreadPool> threadPool() { return threadPool_; }

  /// Starts the server if it's not listening.
  ///
  /// It's harmless to call it multiple times.
  /// Thread safe.
  void start();

  /// Set connection callback.
  /// Not thread safe.
  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  /// Set message callback.
  /// Not thread safe.
  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  /// Set write complete callback.
  /// Not thread safe.
  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }

 private:
  /// Not thread safe, but in loop
  void newConnection(int sockfd, const InetAddress& peerAddr);
  /// Thread safe.
  void removeConnection(const std::shared_ptr<TcpConnection>& conn);
  /// Not thread safe, but in loop
  void removeConnectionInLoop(const std::shared_ptr<TcpConnection>& conn);

  using ConnectionMap = std::map<std::string, std::shared_ptr<TcpConnection>>;

  EventLoop* loop_;  // the acceptor loop
  const std::string ipPort_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;  // avoid revealing Acceptor
  std::shared_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  ThreadInitCallback threadInitCallback_;
  std::atomic<bool> started_{};
  // always in loop thread
  int nextConnId_{1};
  ConnectionMap connections_;
};

}  // namespace net
}  // namespace jinduo
