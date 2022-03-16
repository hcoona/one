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

#include "one/codelab/minikafka/protocol/api_key.h"

#include <type_traits>

#include "absl/strings/str_cat.h"

namespace hcoona {
namespace minikafka {

std::string to_string(ApiKey api_key) {
  switch (api_key) {
    case ApiKey::kMetadata:
      return "Metadata";
    case ApiKey::kApiVersions:
      return "ApiVersions";
    default:
      return absl::StrCat(
          "Unknown(", static_cast<std::underlying_type<ApiKey>::type>(api_key),
          ")");
  }
}

std::ostream& operator<<(std::ostream& os, ApiKey api_key) {
  os << to_string(api_key);
  return os;
}

absl::StatusOr<int16_t> GetRequestHeaderVersion(ApiKey api_key,
                                                int16_t request_api_version) {
  switch (api_key) {
    case ApiKey::kApiVersions:
      if (request_api_version >= 3) {
        return 2;
      } else {
        return 1;
      }
    default:
      return absl::UnimplementedError(
          absl::StrCat("Not implemented yet. api_key=", to_string(api_key)));
  }
}

}  // namespace minikafka
}  // namespace hcoona
