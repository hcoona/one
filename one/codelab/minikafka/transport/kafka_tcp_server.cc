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

#include "one/codelab/minikafka/transport/kafka_tcp_server.h"

#include "absl/functional/bind_front.h"
#include "glog/logging.h"

namespace hcoona {
namespace minikafka {

KafkaTcpServer::KafkaTcpServer(jinduo::net::EventLoop* loop,
                               const jinduo::net::InetAddress& listen_address)
    : loop_(loop),
      tcp_server_(loop, listen_address, "minikafka-server",
                  jinduo::net::TcpServer::kReusePort) {
  tcp_server_.setConnectionCallback(
      absl::bind_front(&KafkaTcpServer::OnConnect, this));
}

void KafkaTcpServer::Stop() {
  loop_->runInLoop(absl::bind_front(&jinduo::net::EventLoop::quit, loop_));
}

void KafkaTcpServer::OnConnect(
    const std::shared_ptr<jinduo::net::TcpConnection>& connection) {
  absl::MutexLock lock(&mutex_);
  if (connection->connected()) {
    auto result = sessions_.try_emplace(
        connection->name(), std::make_shared<KafkaTcpSession>(connection));
    CHECK(result.second);

    VLOG(1) << "Connection established. remote="
            << connection->peerAddress().toIpPort();
  } else {
    CHECK_NE(sessions_.erase(connection->name()), static_cast<size_t>(0));

    VLOG(1) << "Connection destoried. remote="
            << connection->peerAddress().toIpPort();
  }
}

}  // namespace minikafka
}  // namespace hcoona
