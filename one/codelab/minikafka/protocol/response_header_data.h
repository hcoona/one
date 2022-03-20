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
#include "one/codelab/minikafka/base/kafka_binary_writer.h"

namespace hcoona {
namespace minikafka {

class ResponseHeaderData {
  static constexpr int16_t kLowestSupportedVersion = 0;
  static constexpr int16_t kHighestSupportedVersion = 1;

 public:
  [[nodiscard]] static int16_t lowest_supported_version() {
    return kLowestSupportedVersion;
  }

  [[nodiscard]] static int16_t highest_supported_version() {
    return kHighestSupportedVersion;
  }

  [[nodiscard]] int32_t correlation_id() const { return correlation_id_; }
  void set_correlation_id(int32_t correlation_id) {
    correlation_id_ = correlation_id;
  }

  absl::Status WriteTo(KafkaBinaryWriter* writer,
                       int16_t header_version) const {
    // TODO(zhangshuai.ustc): Handle unknown tagged fields.
    (void)header_version;
    return writer->WriteInt32(correlation_id_);
  }

 private:
  std::int32_t correlation_id_{0};

  // TODO(zhangshuai.ustc): Support unknown tagged fields in the future.
};

}  // namespace minikafka
}  // namespace hcoona
