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

#include <iosfwd>
#include <string>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "one/codelab/minikafka/base/kafka_binary_reader.h"
#include "one/codelab/minikafka/protocol/api_key.h"
#include "one/codelab/minikafka/protocol/request_header_data.h"

namespace hcoona {
namespace minikafka {

class RequestHeader {
 public:
  absl::Status ParseFrom(KafkaBinaryReader* reader);

  [[nodiscard]] ApiKey request_api_key() const {
    return static_cast<ApiKey>(data_.request_api_key());
  }
  [[nodiscard]] int16_t request_api_version() const {
    return data_.request_api_version();
  }
  [[nodiscard]] int16_t header_version() const { return header_version_; }
  [[nodiscard]] int32_t correlation_id() const {
    return data_.correlation_id();
  }
  [[nodiscard]] const std::string& client_id() const {
    return data_.client_id();
  }

 private:
  RequestHeaderData data_{};
  int16_t header_version_{};
};

std::ostream& operator<<(std::ostream& os, const RequestHeader& request_header);

}  // namespace minikafka
}  // namespace hcoona
