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
#include <string>
#include <type_traits>

#include "absl/base/casts.h"
#include "absl/base/internal/endian.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"

namespace hcoona {
namespace minikafka {

class KafkaBinaryReader {
 public:
  explicit KafkaBinaryReader(absl::Span<const uint8_t> span)
      : begin_(&span[0]), current_(&span[0]), end_(&span[span.size()]) {}

  absl::Status Read(std::int16_t* value) {
    return Read<std::int16_t, &LoadInt16>(value);
  }

  absl::Status Read(std::int32_t* value) {
    return Read<std::int32_t, &LoadInt32>(value);
  }

  absl::Status ReadString(std::string* value, int32_t length) {
    if (current_ + length <= end_) {
      *value = std::string(current_, current_ + length);
      current_ += length;
      return absl::OkStatus();
    }

    return absl::FailedPreconditionError(absl::StrFormat(
        "There is no enough data for reading. current=%p, end=%p", current_,
        end_));
  }

 private:
  template <typename T, T (*LoaderFn)(const void*)>
  absl::Status Read(T* value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type.");

    if (current_ + sizeof(T) <= end_) {
      *value = LoaderFn(current_);
      current_ += sizeof(*value);
      return absl::OkStatus();
    }

    return absl::FailedPreconditionError(absl::StrFormat(
        "There is no enough data for reading. current=%p, end=%p", current_,
        end_));
  }

  static std::int16_t LoadInt16(const void* p) {
    return absl::bit_cast<std::int16_t>(absl::little_endian::Load16(p));
  }

  static std::int32_t LoadInt32(const void* p) {
    return absl::bit_cast<std::int32_t>(absl::little_endian::Load32(p));
  }

  const std::uint8_t* begin_{nullptr};
  const std::uint8_t* current_{nullptr};
  const std::uint8_t* end_{nullptr};
};

}  // namespace minikafka
}  // namespace hcoona
