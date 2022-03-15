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

#include "one/codelab/minikafka/request_header.h"

#include <string>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "one/base/macros.h"
#include "one/codelab/minikafka/api_key.h"
#include "one/codelab/minikafka/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

absl::Status RequestHeader::ParseFrom(
    absl::Span<const uint8_t> request_header_bytes) {
  static constexpr size_t kRequestApiKeySize = sizeof(int16_t);
  static constexpr size_t kRequestApiVersionSize = sizeof(int16_t);

  if (request_header_bytes.size() <
      kRequestApiKeySize + kRequestApiVersionSize) {
    return absl::InvalidArgumentError(
        "request_header_bytes too few to parse ApiKey & ApiVersion");
  }

  KafkaBinaryReader kafka_binary_reader(request_header_bytes);

  int16_t api_key_number;
  ONE_RETURN_IF_NOT_OK(kafka_binary_reader.ReadBe(&api_key_number));
  int16_t api_version;
  ONE_RETURN_IF_NOT_OK(kafka_binary_reader.ReadBe(&api_version));

  auto api_key = static_cast<ApiKey>(api_key_number);
  ONE_ASSIGN_OR_RETURN(header_version_,
                       GetRequestHeaderVersion(api_key, api_version));

  ONE_RETURN_IF_NOT_OK(data_.ParseFrom(request_header_bytes, header_version_));

  return absl::OkStatus();
}

std::ostream& operator<<(std::ostream& os,
                         const RequestHeader& request_header) {
  os << "RequestHeader(api_key=" << request_header.request_api_key()
     << ",api_version=" << request_header.request_api_version()
     << ",header_version=" << request_header.header_version()
     << ",correlation_id=" << request_header.correlation_id()
     << ",client_id=" << request_header.client_id() << ")";
  return os;
}

}  // namespace minikafka
}  // namespace hcoona
