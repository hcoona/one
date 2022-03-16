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

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/base/casts.h"
#include "absl/base/internal/endian.h"
#include "absl/base/optimization.h"
#include "absl/status/status.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "glog/logging.h"
#include "one/base/macros.h"

namespace hcoona {
namespace minikafka {

// Read https://kafka.apache.org/protocol#protocol_types from underlying bytes.
class KafkaBinaryReader {
  // TODO(zhangshuai.ustc): any reading must be verified not exceeding the
  // `end_` position.

  static constexpr uint8_t kVarintMostSignificantBitMask = 0x80U;

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

  absl::Status ReadInt16(std::int16_t* value) {
    return Read<std::int16_t, &LoadInt16Be>(value);
  }

  absl::Status ReadInt32(std::int32_t* value) {
    return Read<std::int32_t, &LoadInt32Be>(value);
  }

  ABSL_ATTRIBUTE_ALWAYS_INLINE absl::Status ReadVarint32(std::uint32_t* value) {
    uint32_t v = 0;
    if (ABSL_PREDICT_TRUE(current_ < end_)) {
      v = *current_;
      if (v < kVarintMostSignificantBitMask) {
        *value = v;
        current_++;
        return absl::OkStatus();
      }
    }
    int64_t result = ReadVarint32Fallback(v);
    *value = static_cast<uint32_t>(result);
    if (result < 0) {
      static constexpr size_t kVarintEncodedBytesMax = 10;
      return absl::UnknownError(
          absl::StrCat("Failed to read varint value. HEX=",
                       absl::BytesToHexString(
                           {reinterpret_cast<const char*>(current_),
                            std::min(kVarintEncodedBytesMax,
                                     static_cast<size_t>(end_ - current_))})));
    }

    return absl::OkStatus();
  }

  ABSL_ATTRIBUTE_ALWAYS_INLINE absl::Status ReadVarint64(std::uint64_t* value) {
    if (ABSL_PREDICT_TRUE(current_ < end_) &&
        *current_ < kVarintMostSignificantBitMask) {
      *value = *current_;
      current_++;
      return absl::OkStatus();
    }
    std::pair<uint64_t, bool> p = ReadVarint64Fallback();
    *value = p.first;
    if (!p.second) {
      static constexpr size_t kVarintEncodedBytesMax = 10;
      return absl::UnknownError(
          absl::StrCat("Failed to read varint value. HEX=",
                       absl::BytesToHexString(
                           {reinterpret_cast<const char*>(current_),
                            std::min(kVarintEncodedBytesMax,
                                     static_cast<size_t>(end_ - current_))})));
    }
    return absl::OkStatus();
  }

  absl::Status ReadString(std::string* value, int32_t length) {
    CHECK_GE(length, 0);

    if (current_ + length <= end_) {
      *value = std::string(current_, current_ + length);
      current_ += length;
      return absl::OkStatus();
    }

    return absl::FailedPreconditionError(absl::StrFormat(
        "There is no enough data for reading. current=%p, end=%p, length=%d",
        current_, end_, length));
  }

  // Represents a sequence of characters. First the length N is given as an
  // INT16. Then N bytes follow which are the UTF-8 encoding of the character
  // sequence. Length must not be negative.
  absl::Status ReadString(std::string* value) {
    return ReadString(value, /*nullable=*/false);
  }

  // Represents a sequence of characters or null. For non-null strings, first
  // the length N is given as an INT16. Then N bytes follow which are the UTF-8
  // encoding of the character sequence. A null value is encoded with length of
  // -1 and there are no following bytes.
  absl::Status ReadNullableString(std::string* value) {
    return ReadString(value, /*nullable=*/true);
  }

  // Represents a sequence of characters. First the length N + 1 is given as an
  // UNSIGNED_VARINT . Then N bytes follow which are the UTF-8 encoding of the
  // character sequence.
  absl::Status ReadCompactString(std::string* value) {
    return ReadCompactString(value, /*nullable=*/false,
                             std::numeric_limits<int32_t>::max());
  }

  // Represents a sequence of characters. First the length N + 1 is given as an
  // UNSIGNED_VARINT . Then N bytes follow which are the UTF-8 encoding of the
  // character sequence. A null string is represented with a length of 0.
  absl::Status ReadNullableCompactString(std::string* value) {
    return ReadCompactString(value, /*nullable=*/true,
                             std::numeric_limits<int32_t>::max());
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

  absl::Status ReadString(std::string* value, bool nullable) {
    int16_t length;
    ONE_RETURN_IF_NOT_OK(ReadInt16(&length));
    if (length < 0) {
      if (nullable) {
        CHECK_EQ(-1, length);
        value->clear();
        return absl::OkStatus();
      }
      return absl::UnknownError("String length is less than 0");
    }
    return ReadString(value, length);
  }

  absl::Status ReadCompactString(std::string* value, bool nullable,
                                 std::optional<int64_t> length_max) {
    uint32_t length_plus_one;
    ONE_RETURN_IF_NOT_OK(ReadVarint32(&length_plus_one));
    int64_t length = static_cast<int64_t>(length_plus_one) - 1;
    if (length < 0) {
      return absl::UnknownError("CompactString length is less than 0");
    }
    if (length == 0) {
      if (nullable) {
        value->clear();
        return absl::OkStatus();
      }
      return absl::UnknownError("Non-nullable CompactString length is 0");
    }
    if (length_max.has_value() && length > length_max.value()) {
      return absl::UnknownError("String length is larger than limit");
    }
    CHECK_LE(length, std::numeric_limits<int32_t>::max());
    return ReadString(value, static_cast<int32_t>(length));
  }

  const std::uint8_t* begin_{nullptr};
  const std::uint8_t* current_{nullptr};
  const std::uint8_t* end_{nullptr};

  std::vector<const std::uint8_t*> recorded_positions_{};
};

}  // namespace minikafka
}  // namespace hcoona
