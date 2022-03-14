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
#include <utility>

#include "absl/time/time.h"
#include "one/jinduo/net/buffer.h"
#include "one/jinduo/net/tcp_connection.h"

namespace hcoona {
namespace minikafka {

class KafkaTcpSession {
 public:
  explicit KafkaTcpSession(
      std::shared_ptr<jinduo::net::TcpConnection> connection);

 private:
  void OnMessage(const std::shared_ptr<jinduo::net::TcpConnection>& connection,
                 jinduo::net::Buffer* buffer, absl::Time receiveTime);

  std::shared_ptr<jinduo::net::TcpConnection> connection_;
};

}  // namespace minikafka
}  // namespace hcoona
