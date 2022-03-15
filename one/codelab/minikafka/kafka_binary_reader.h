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
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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
      : begin_(span.data()),
        current_(span.data()),
        end_(span.data() + span.size()) {}

  void RecordCurrentPosition() { recorded_positions_.emplace_back(current_); }

  void RewindRecordedPosition() {
    current_ = recorded_positions_.back();
    recorded_positions_.pop_back();
  }

  void ClearRecordedPosition() { recorded_positions_.clear(); }

  absl::Status ReadLe(std::int16_t* value) {
    return Read<std::int16_t, &LoadInt16Le>(value);
  }

  absl::Status ReadLe(std::int32_t* value) {
    return Read<std::int32_t, &LoadInt32Le>(value);
  }

  absl::Status ReadBe(std::int16_t* value) {
    return Read<std::int16_t, &LoadInt16Be>(value);
  }

  absl::Status ReadBe(std::int32_t* value) {
    return Read<std::int32_t, &LoadInt32Be>(value);
  }

  bool ReadVarint32(std::uint32_t* value);
  bool ReadVarint64(std::uint64_t* value);

  absl::Status ReadString(std::string* value, int32_t length) {
    if (current_ + length <= end_) {
      *value = std::string(current_, current_ + length);
      current_ += length;
      return absl::OkStatus();
    }

    return absl::FailedPreconditionError(absl::StrFormat(
        "There is no enough data for reading. current=%p, end=%p, length=%d",
        current_, end_, length));
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

  static std::int16_t LoadInt16Le(const void* p) {
    return absl::bit_cast<std::int16_t>(absl::little_endian::Load16(p));
  }

  static std::int32_t LoadInt32Le(const void* p) {
    return absl::bit_cast<std::int32_t>(absl::little_endian::Load32(p));
  }

  static std::int16_t LoadInt16Be(const void* p) {
    return absl::bit_cast<std::int16_t>(absl::big_endian::Load16(p));
  }

  static std::int32_t LoadInt32Be(const void* p) {
    return absl::bit_cast<std::int32_t>(absl::big_endian::Load32(p));
  }

  bool ReadVarint32Slow(uint32_t* value);
  int64_t ReadVarint32Fallback(uint32_t first_byte_or_zero);
  bool ReadVarint64Slow(uint64_t* value);
  std::pair<uint64_t, bool> ReadVarint64Fallback();

  const std::uint8_t* begin_{nullptr};
  const std::uint8_t* current_{nullptr};
  const std::uint8_t* end_{nullptr};

  std::vector<const std::uint8_t*> recorded_positions_{};
};

}  // namespace minikafka
}  // namespace hcoona
