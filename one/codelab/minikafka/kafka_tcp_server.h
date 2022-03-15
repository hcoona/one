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

#pragma once

#include <memory>
#include <string>

#include "absl/base/thread_annotations.h"
#include "absl/container/flat_hash_map.h"
#include "one/codelab/minikafka/kafka_tcp_session.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/inet_address.h"
#include "one/jinduo/net/tcp_connection.h"
#include "one/jinduo/net/tcp_server.h"

namespace hcoona {
namespace minikafka {

class KafkaTcpServer {
 public:
  KafkaTcpServer(jinduo::net::EventLoop* loop,
                 const jinduo::net::InetAddress& listen_address);
  virtual ~KafkaTcpServer() = default;

  // Disallow copy.
  KafkaTcpServer(const KafkaTcpServer&) noexcept = delete;
  KafkaTcpServer& operator=(const KafkaTcpServer&) noexcept = delete;

  // Allow move but not implemented yet.
  KafkaTcpServer(KafkaTcpServer&&) noexcept = delete;
  KafkaTcpServer& operator=(KafkaTcpServer&&) noexcept = delete;

  void Start() { tcp_server_.start(); }
  void Stop();

  const std::string& GetIpPort() const { return tcp_server_.ipPort(); }

 private:
  void OnConnect(const std::shared_ptr<jinduo::net::TcpConnection>& connection);

  jinduo::net::EventLoop* loop_;
  jinduo::net::TcpServer tcp_server_;

  mutable absl::Mutex mutex_;
  absl::flat_hash_map<std::string, std::shared_ptr<KafkaTcpSession>> sessions_
      ABSL_GUARDED_BY(mutex_);
};

}  // namespace minikafka
}  // namespace hcoona
