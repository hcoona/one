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
#include <variant>
#include <vector>

#include "absl/time/time.h"
#include "one/codelab/minikafka/protocol/api_versions_request.h"
#include "one/codelab/minikafka/protocol/request_header.h"
#include "one/jinduo/net/tcp_connection.h"

namespace hcoona {
namespace minikafka {

struct RequestHeaderAndBody {
  RequestHeader header;
  std::variant<std::monostate, ApiVersionsRequest> body;
  absl::Time receive_time;
};

class KafkaService {
 public:
  // TODO(zhangshuai.ds): dispatch into different processing queues.
  //
  // 1. Metadata
  // 2. Producing
  // 3. Consuming
  // 4. Coordinator
  // 5. Administrator
  void PostRequests(std::weak_ptr<jinduo::net::TcpConnection> connection,
                    std::vector<RequestHeaderAndBody>&& requests);

 private:
  // TODO(zhangshuai.ds): process different API requests in arbitrary order, but
  // send response in origin ordering.
  static void ProcessApiVersionsRequest(
      std::weak_ptr<jinduo::net::TcpConnection> connection,
      RequestHeaderAndBody&& request_header_and_body);
};

}  // namespace minikafka
}  // namespace hcoona
