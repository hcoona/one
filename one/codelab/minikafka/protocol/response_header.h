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

#include "absl/status/status.h"
#include "one/codelab/minikafka/base/kafka_binary_writer.h"
#include "one/codelab/minikafka/protocol/response_header_data.h"

namespace hcoona {
namespace minikafka {

class ResponseHeader {
 public:
  ResponseHeader() = default;
  ResponseHeader(int32_t correlation_id, int16_t header_version)
      : header_version_(header_version) {
    data_.set_correlation_id(correlation_id);
  }

  absl::Status WriteTo(KafkaBinaryWriter* writer) {
    return data_.WriteTo(writer, header_version_);
  }

  [[nodiscard]] int16_t header_version() const { return header_version_; }
  void set_header_version(int16_t header_version) {
    header_version_ = header_version;
  }

  [[nodiscard]] int32_t correlation_id() const {
    return data_.correlation_id();
  }
  void set_correlation_id(int32_t correlation_id) {
    data_.set_correlation_id(correlation_id);
  }

 private:
  ResponseHeaderData data_{};
  int16_t header_version_{};
};

}  // namespace minikafka
}  // namespace hcoona
