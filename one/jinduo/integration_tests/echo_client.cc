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

#include <memory>
#include <utility>
#include <vector>

#include "absl/base/casts.h"
#include "absl/functional/bind_front.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/inet_address.h"
#include "one/jinduo/net/tcp_client.h"

using jinduo::net::Buffer;
using jinduo::net::EventLoop;
using jinduo::net::InetAddress;
using jinduo::net::TcpClient;
using jinduo::net::TcpConnection;

int numThreads = 0;
class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;
int current = 0;

class EchoClient {
 public:
  EchoClient(EventLoop* loop, const InetAddress& listenAddr,
             const std::string& id)
      : loop_(loop), client_(loop, listenAddr, "EchoClient" + id) {
    client_.setConnectionCallback(&EchoClient::onConnection);
    client_.setMessageCallback(absl::bind_front(&EchoClient::onMessage, this));
    // client_.enableRetry();
  }
  virtual ~EchoClient() = default;

  // Disallow copy.
  EchoClient(const EchoClient&) noexcept = delete;
  EchoClient& operator=(const EchoClient&) noexcept = delete;

  // Allow move but not implemented yet.
  EchoClient(EchoClient&&) noexcept = delete;
  EchoClient& operator=(EchoClient&&) noexcept = delete;

  void connect() { client_.connect(); }
  // void stop();

 private:
  static void onConnection(const std::shared_ptr<TcpConnection>& conn) {
    VLOG(1) << conn->localAddress().toIpPort() << " -> "
            << conn->peerAddress().toIpPort() << " is "
            << (conn->connected() ? "UP" : "DOWN");

    if (conn->connected()) {
      ++current;
      if (absl::implicit_cast<size_t>(current) < clients.size()) {
        clients[current]->connect();
      }
      LOG(INFO) << "*** connected " << current;
    }
    conn->send("world\n");
  }

  void onMessage(const std::shared_ptr<TcpConnection>& conn, Buffer* buf,
                 absl::Time time) {
    std::string msg(buf->retrieveAllAsString());
    VLOG(1) << conn->name() << " recv " << msg.size() << " bytes at " << time;
    if (msg == "quit\n") {
      conn->send("bye\n");
      conn->shutdown();
    } else if (msg == "shutdown\n") {
      loop_->Quit();
    } else {
      conn->send(msg);
    }
  }

  EventLoop* loop_;
  TcpClient client_;
};

int main(int argc, char* argv[]) {
  LOG(INFO) << "pid = " << getpid() << ", tid = " << jinduo::this_thread::tid();
  if (argc > 1) {
    EventLoop loop;
    bool ipv6 = argc > 3;
    InetAddress serverAddr(argv[1], 2000, ipv6);

    int n = 1;
    if (argc > 2) {
      n = atoi(argv[2]);
    }

    clients.reserve(n);
    for (int i = 0; i < n; ++i) {
      char buf[32];
      snprintf(buf, sizeof buf, "%d", i + 1);
      clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
    }

    clients[current]->connect();
    loop.Loop();
  } else {
    printf("Usage: %s host_ip [current#]\n", argv[0]);
  }
}
