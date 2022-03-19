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

#include <stdio.h>
#include <unistd.h>

#include <utility>

#include "absl/functional/bind_front.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/inet_address.h"
#include "one/jinduo/net/tcp_server.h"

using jinduo::net::Buffer;
using jinduo::net::EventLoop;
using jinduo::net::InetAddress;
using jinduo::net::TcpConnection;
using jinduo::net::TcpServer;

int numThreads = 0;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
      : loop_(loop), server_(loop, listenAddr, "EchoServer") {
    server_.setConnectionCallback(&EchoServer::onConnection);
    server_.setMessageCallback(absl::bind_front(&EchoServer::onMessage, this));
    server_.setThreadNum(numThreads);
  }

  void start() { server_.start(); }
  // void stop();

 private:
  static void onConnection(const std::shared_ptr<TcpConnection>& conn) {
    VLOG(1) << conn->peerAddress().toIpPort() << " -> "
            << conn->localAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");
    LOG(INFO) << conn->getTcpInfoString();

    conn->send("hello\n");
  }

  void onMessage(const std::shared_ptr<TcpConnection>& conn, Buffer* buf,
                 absl::Time time) {
    std::string msg(buf->retrieveAllAsString());
    VLOG(1) << conn->name() << " recv " << msg.size() << " bytes at " << time;
    if (msg == "exit\n") {
      conn->send("bye\n");
      conn->shutdown();
    }
    if (msg == "quit\n") {
      loop_->Quit();
    }
    conn->send(msg);
  }

  EventLoop* loop_;
  TcpServer server_;
};

int main(int argc, char* argv[]) {
  LOG(INFO) << "pid = " << getpid() << ", tid = " << jinduo::this_thread::tid();
  LOG(INFO) << "sizeof TcpConnection = " << sizeof(TcpConnection);
  if (argc > 1) {
    numThreads = atoi(argv[1]);
  }
  bool ipv6 = argc > 2;
  EventLoop loop;
  InetAddress listenAddr(2000, /*loopbackOnly=*/false, ipv6);
  EchoServer server(&loop, listenAddr);

  server.start();

  loop.Loop();
}
