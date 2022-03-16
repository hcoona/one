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

#include <cstdint>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "one/codelab/minikafka/base/kafka_binary_reader.h"

namespace hcoona {
namespace minikafka {

class ApiVersionsRequestData {
  static constexpr int16_t kLowestSupportedVersion = 0;
  static constexpr int16_t kHighestSupportedVersion = 3;

 public:
  [[nodiscard]] static int16_t lowest_supported_version() {
    return kLowestSupportedVersion;
  }

  [[nodiscard]] static int16_t highest_supported_version() {
    return kHighestSupportedVersion;
  }

  [[nodiscard]] const std::string& client_software_name() const {
    return client_software_name_;
  }
  void set_client_software_name(std::string client_software_name) {
    client_software_name_ = std::move(client_software_name);
  }

  [[nodiscard]] const std::string& client_software_version() const {
    return client_software_version_;
  }
  void set_client_software_version(std::string client_software_version) {
    client_software_version_ = std::move(client_software_version);
  }

  absl::Status ParseFrom(KafkaBinaryReader* reader, int16_t api_version);

 private:
  std::string client_software_name_;
  std::string client_software_version_;

  // TODO(zhangshuai.ustc): Support unknown tagged fields in the future.
};

}  // namespace minikafka
}  // namespace hcoona
