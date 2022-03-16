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

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>
#include <type_traits>

#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"

namespace hcoona {
namespace minikafka {

enum class ApiKey : int16_t {
  kProduce = 0,
  kFetch = 1,
  kListOffsets = 2,
  kMetadata = 3,
  kSaslHandshake = 17,
  kApiVersions = 18,
};

constexpr size_t kApiKeyCount = 68;
constexpr std::underlying_type<ApiKey>::type kApiKeyValueMin = 0;
constexpr std::underlying_type<ApiKey>::type kApiKeyValueMax = 67;

std::string to_string(ApiKey api_key);

std::ostream& operator<<(std::ostream& os, ApiKey api_key);

absl::StatusOr<int16_t> GetRequestHeaderVersion(ApiKey api_key,
                                                int16_t request_api_version);

}  // namespace minikafka
}  // namespace hcoona
