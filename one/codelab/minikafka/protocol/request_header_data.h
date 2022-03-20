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

class RequestHeaderData {
  static constexpr int16_t kLowestSupportedVersion = 0;
  static constexpr int16_t kHighestSupportedVersion = 1;

 public:
  [[nodiscard]] static int16_t lowest_supported_version() {
    return kLowestSupportedVersion;
  }

  [[nodiscard]] static int16_t highest_supported_version() {
    return kHighestSupportedVersion;
  }

  [[nodiscard]] int16_t api_key() const { return api_key_; }
  void set_api_key(std::int16_t api_key) { api_key_ = api_key; }

  [[nodiscard]] int16_t api_version() const { return api_version_; }
  void set_api_version(int16_t api_version) { api_version_ = api_version; }

  [[nodiscard]] int32_t correlation_id() const { return correlation_id_; }
  void correlation_id(int32_t correlation_id) {
    correlation_id_ = correlation_id;
  }

  [[nodiscard]] const std::string& client_id() const { return client_id_; }
  void set_client_id(std::string client_id) {
    client_id_ = std::move(client_id);
  }

  absl::Status ParseFrom(KafkaBinaryReader* reader, int16_t header_version);

 private:
  std::int16_t api_key_{0};
  std::int16_t api_version_{0};
  std::int32_t correlation_id_{0};
  std::string client_id_;

  // TODO(zhangshuai.ustc): Support unknown tagged fields in the future.
};

}  // namespace minikafka
}  // namespace hcoona
