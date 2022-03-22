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

#include <utility>
#include <vector>

#include "one/codelab/minikafka/protocol/api_versions_response_data.h"

namespace hcoona {
namespace minikafka {

class ApiVersionsResponse {
 public:
  ApiVersionsResponse() = default;
  ApiVersionsResponse(
      int32_t throttle_time_ms,
      std::vector<ApiVersionsResponseData::ApiVersionEntry> api_versions) {
    data_.set_throttle_time_ms(throttle_time_ms);
    data_.set_api_keys(std::move(api_versions));
  }

  absl::Status WriteTo(KafkaBinaryWriter* writer, int16_t api_version) const {
    return data_.WriteTo(writer, api_version);
  }

 private:
  ApiVersionsResponseData data_;
};

}  // namespace minikafka
}  // namespace hcoona
