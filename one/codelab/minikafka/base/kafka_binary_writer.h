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
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/base/attributes.h"
#include "absl/base/internal/endian.h"
#include "absl/base/optimization.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "one/base/macros.h"

namespace hcoona {
namespace minikafka {

// Write https://kafka.apache.org/protocol#protocol_types from underlying bytes.
class KafkaBinaryWriter {
  static constexpr uint8_t kVarintMostSignificantBitMask = 0x80U;
  static constexpr uint32_t kBitNumWithoutMostSignificantBitInOneByte = 7;

 public:
  explicit KafkaBinaryWriter(absl::Span<uint8_t> span)
      : begin_(span.data()),
        current_(span.data()),
        end_(span.data() + span.size()) {}

  [[nodiscard]] size_t size() const { return current_ - begin_; }

  absl::Status WriteInt16(std::int16_t value) {
    if (current_ + sizeof(value) > end_) {
      return absl::FailedPreconditionError(absl::StrFormat(
          "There is no enough space for writing. current=%p, end=%p, length=%d",
          current_, end_, sizeof(value)));
    }

    return Write(absl::big_endian::FromHost(value));
  }

  absl::Status WriteInt32(std::int32_t value) {
    if (current_ + sizeof(value) > end_) {
      return absl::FailedPreconditionError(absl::StrFormat(
          "There is no enough space for writing. current=%p, end=%p, length=%d",
          current_, end_, sizeof(value)));
    }

    return Write(absl::big_endian::FromHost(value));
  }

  absl::Status WriteVarint32(std::uint32_t value) {
    if (current_ + sizeof(value) > end_) {
      return absl::FailedPreconditionError(absl::StrFormat(
          "There is no enough space for writing. current=%p, end=%p, length=%d",
          current_, end_, sizeof(value)));
    }

    current_ = UnsafeVarint(value, current_);
    return absl::OkStatus();
  }

  template <typename Container>
  absl::Status WriteRawArray(const Container& c) {
    static_assert(sizeof(typename Container::value_type) == sizeof(uint8_t),
                  "Container value_type must has same size to one byte.");
    static_assert(std::is_trivial<typename Container::value_type>::value,
                  "Container value_type must be a trivial type.");

    if (current_ + c.size() > end_) {
      return absl::FailedPreconditionError(absl::StrFormat(
          "There is no enough space for writing. current=%p, end=%p, length=%d",
          current_, end_, c.size()));
    }

    ::memcpy(current_, c.data(), c.size());
    current_ += c.size();

    return absl::OkStatus();
  }

  // Represents a sequence of characters. First the length N is given as an
  // INT16. Then N bytes follow which are the UTF-8 encoding of the character
  // sequence. Length must not be negative.
  absl::Status WriteString(const std::string& value) {
    if (value.empty()) {
      return absl::InvalidArgumentError("value must be non-empty string.");
    }

    return WriteStringInternal(value);
  }

  // Represents a sequence of characters or null. For non-null strings, first
  // the length N is given as an INT16. Then N bytes follow which are the UTF-8
  // encoding of the character sequence. A null value is encoded with length of
  // -1 and there are no following bytes.
  absl::Status WriteNullableString(const std::string& value) {
    return WriteStringInternal(value);
  }

  // Represents a sequence of characters. First the length N + 1 is given as an
  // UNSIGNED_VARINT . Then N bytes follow which are the UTF-8 encoding of the
  // character sequence.
  absl::Status WriteCompactString(const std::string& value) {
    if (value.empty()) {
      return absl::InvalidArgumentError("value must be non-empty string.");
    }

    return WriteCompactStringInternal(value);
  }

  // Represents a sequence of characters. First the length N + 1 is given as an
  // UNSIGNED_VARINT . Then N bytes follow which are the UTF-8 encoding of the
  // character sequence. A null string is represented with a length of 0.
  absl::Status WriteNullableCompactString(const std::string& value) {
    return WriteCompactStringInternal(value);
  }

 private:
  template <typename T>
  absl::Status Write(T value) {
    static_assert(std::is_integral<T>::value, "T must be an integral type.");

    if (current_ + sizeof(T) > end_) {
      return absl::FailedPreconditionError(absl::StrFormat(
          "There is no enough data for writing. current=%p, end=%p", current_,
          end_));
    }
    ::memcpy(current_, &value, sizeof(T));
    current_ += sizeof(T);
    return absl::OkStatus();
  }

  // Represents a sequence of characters or null. For non-null strings, first
  // the length N is given as an INT16. Then N bytes follow which are the UTF-8
  // encoding of the character sequence. A null value is encoded with length of
  // -1 and there are no following bytes.
  absl::Status WriteStringInternal(const std::string& value) {
    if (value.empty()) {
      static constexpr int16_t kNullStringLengthPrefix = -1;
      return WriteInt16(kNullStringLengthPrefix);
    }

    if (value.size() >
        static_cast<size_t>(std::numeric_limits<int16_t>::max())) {
      return absl::InvalidArgumentError(
          "value is too long, which size is larger than INT16_MAX");
    }

    ONE_RETURN_IF_NOT_OK(WriteInt16(static_cast<int16_t>(value.size())));
    return WriteRawArray(value);
  }

  // Represents a sequence of characters. First the length N + 1 is given as an
  // UNSIGNED_VARINT . Then N bytes follow which are the UTF-8 encoding of the
  // character sequence. A null string is represented with a length of 0.
  absl::Status WriteCompactStringInternal(const std::string& value) {
    if (value.empty()) {
      static constexpr uint32_t kNullStringLengthPrefix = 0;
      return WriteVarint32(kNullStringLengthPrefix);
    }

    if (value.size() + 1 >
        static_cast<size_t>(std::numeric_limits<uint32_t>::max())) {
      return absl::InvalidArgumentError(
          "value is too long, which size is larger than UINT32_MAX");
    }

    ONE_RETURN_IF_NOT_OK(WriteVarint32(value.size() + 1));
    return WriteRawArray(value);
  }

  // Vendor writing varint methods from protobuf source code.
  //
  // Copied from
  // https://github.com/protocolbuffers/protobuf/blob/7ecf43f0cedc4320c1cb31ba787161011b62e741/src/google/protobuf/io/coded_stream.cc
  //
  // Protocol Buffers - Google's data interchange format
  // Copyright 2008 Google Inc.  All rights reserved.
  // https://developers.google.com/protocol-buffers/
  //
  // Redistribution and use in source and binary forms, with or without
  // modification, are permitted provided that the following conditions are
  // met:
  //
  //     * Redistributions of source code must retain the above copyright
  // notice, this list of conditions and the following disclaimer.
  //     * Redistributions in binary form must reproduce the above
  // copyright notice, this list of conditions and the following disclaimer
  // in the documentation and/or other materials provided with the
  // distribution.
  //     * Neither the name of Google Inc. nor the names of its
  // contributors may be used to endorse or promote products derived from
  // this software without specific prior written permission.
  //
  // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  // A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  // OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  // SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  // LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  // DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  // THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  // (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  // OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  template <typename T>
  ABSL_ATTRIBUTE_ALWAYS_INLINE static uint8_t* UnsafeVarint(T value,
                                                            uint8_t* ptr) {
    static_assert(std::is_unsigned<T>::value,
                  "Varint serialization must be unsigned");
    ptr[0] = static_cast<uint8_t>(value);
    if (value < kVarintMostSignificantBitMask) {
      return ptr + 1;
    }
    // Turn on continuation bit in the byte we just wrote.
    ptr[0] |= static_cast<uint8_t>(kVarintMostSignificantBitMask);
    value >>= kBitNumWithoutMostSignificantBitInOneByte;
    ptr[1] = static_cast<uint8_t>(value);
    if (value < kVarintMostSignificantBitMask) {
      return ptr + 2;
    }
    ptr += 2;
    do {
      // Turn on continuation bit in the byte we just wrote.
      ptr[-1] |= static_cast<uint8_t>(kVarintMostSignificantBitMask);
      value >>= kBitNumWithoutMostSignificantBitInOneByte;
      *ptr = static_cast<uint8_t>(value);
      ++ptr;
    } while (value >= kVarintMostSignificantBitMask);
    return ptr;
  }

  std::uint8_t* begin_{nullptr};
  std::uint8_t* current_{nullptr};
  std::uint8_t* end_{nullptr};
};

}  // namespace minikafka
}  // namespace hcoona
