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

#include <ostream>
#include <regex>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "one/codelab/minikafka/api_key.h"
#include "one/codelab/minikafka/api_versions_request_data.h"
#include "one/codelab/minikafka/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

class ApiVersionsRequest {
  static inline const std::regex kSoftwareNameVersionPattern{
      "[a-zA-Z0-9](?:[a-zA-Z0-9\\-.]*[a-zA-Z0-9])?"};

 public:
  absl::Status ParseFrom(KafkaBinaryReader* reader, int16_t api_version) {
    api_version_ = api_version;
    return data_.ParseFrom(reader, api_version);
  }

  [[nodiscard]] const std::string& client_software_name() const {
    return data_.client_software_name();
  }
  [[nodiscard]] const std::string& client_software_version() const {
    return data_.client_software_version();
  }

  [[nodiscard]] bool validate() const {
    if (api_version_ >= 3) {
      return std::regex_match(client_software_name(),
                              kSoftwareNameVersionPattern) &&
             std::regex_match(client_software_version(),
                              kSoftwareNameVersionPattern);
    }
    return true;
  }

 private:
  ApiVersionsRequestData data_{};
  int16_t api_version_{};
};

inline std::ostream& operator<<(std::ostream& os,
                                const ApiVersionsRequest& request) {
  os << "ApiVersionsRequest(client_software_name="
     << request.client_software_name()
     << ",client_software_version=" << request.client_software_version() << ")";
  return os;
}

}  // namespace minikafka
}  // namespace hcoona
