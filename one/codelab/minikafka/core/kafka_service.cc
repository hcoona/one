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

#include "one/codelab/minikafka/core/kafka_service.h"

#include <limits>
#include <utility>

#include "one/codelab/minikafka/base/kafka_binary_writer.h"
#include "one/codelab/minikafka/protocol/api_versions_response.h"
#include "one/codelab/minikafka/protocol/response_header.h"

namespace hcoona {
namespace minikafka {

void KafkaService::PostRequests(
    std::weak_ptr<jinduo::net::TcpConnection> weak_connection,
    std::vector<RequestHeaderAndBody>&& requests) {
  if (weak_connection.expired()) {
    return;
  }

  for (auto&& request : requests) {
    switch (request.header.api_key()) {
      case ApiKey::kApiVersions:
        ProcessApiVersionsRequest(std::move(weak_connection),
                                  std::move(request));
        break;
      default: {
        LOG(ERROR) << "Unimplemented for API. api_key="
                   << request.header.api_key();
        if (std::shared_ptr<jinduo::net::TcpConnection> connection =
                weak_connection.lock()) {
          connection->shutdown();
        }
        return;
      }
    }
  }
}

// static
void KafkaService::ProcessApiVersionsRequest(
    // NOLINTNEXTLINE(performance-unnecessary-value-param): by-design
    std::weak_ptr<jinduo::net::TcpConnection> weak_connection,
    RequestHeaderAndBody&& request_header_and_body) {
  std::shared_ptr<jinduo::net::TcpConnection> connection =
      weak_connection.lock();
  if (!connection) {
    return;
  }

  auto* body = std::get_if<ApiVersionsRequest>(&request_header_and_body.body);
  (void)body;

  // TODO(zhangshuai.ds): accelerate the size estimation.
  static constexpr size_t kLargeEnoughBufferSize = 65535;
  static constexpr size_t kHeaderStartPosition = sizeof(int32_t);
  auto buffer = std::make_unique<uint8_t[]>(kLargeEnoughBufferSize);
  KafkaBinaryWriter writer(
      absl::MakeSpan(buffer.get() + kHeaderStartPosition,
                     kLargeEnoughBufferSize - kHeaderStartPosition));
  ResponseHeader response_header(
      request_header_and_body.header.correlation_id(),
      request_header_and_body.header.header_version());
  ApiVersionsResponse response_body(
      0, {ApiVersionsResponseData::ApiVersionEntry{
             .api_key = static_cast<int16_t>(ApiKey::kApiVersions),
             .min_version = 0,
             .max_version = 3}});

  absl::Status s = response_header.WriteTo(&writer);
  if (!s.ok()) {
    LOG(ERROR) << "Failed to serialize response_header. reason=" << s;
    connection->shutdown();
    return;
  }
  s = response_body.WriteTo(&writer,
                            request_header_and_body.header.api_version());
  if (!s.ok()) {
    LOG(ERROR) << "Failed to serialize response_body. reason=" << s;
    connection->shutdown();
    return;
  }
  if (writer.size() > std::numeric_limits<int32_t>::max()) {
    LOG(ERROR) << "Failed to serialize response. header+body size is "
                  "larger than INT32_MAX.";
    connection->shutdown();
    return;
  }

  absl::big_endian::Store32(buffer.get(), writer.size());

  CHECK_LE(writer.size() + kHeaderStartPosition,
           static_cast<size_t>(std::numeric_limits<int>::max()));

  connection->send(buffer.get(),
                   static_cast<int>(writer.size() + kHeaderStartPosition));
}

}  // namespace minikafka
}  // namespace hcoona
