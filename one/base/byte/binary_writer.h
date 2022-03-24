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
//
// -----------------------------------------------------------------------------
// File: binary_writer.h
// -----------------------------------------------------------------------------
//
// This header file defines a writer class help write typed data to underlying
// bytes span.

#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "absl/base/internal/endian.h"
#include "one/base/container/span.h"
#include "one/base/macros.h"

namespace hcoona {

template <typename T>
class BinaryWriter {
  static_assert(sizeof(T) == 1 && std::is_scalar<T>::value,
                "T must be a scalar type which size is same as a byte.");

 public:
  explicit BinaryWriter(span<T> buffer) : buffer_(buffer) {}

  // Implicit copy & move are allowed.

  //
  // Element access
  //

  span<T> underlying_buffer() { return buffer_; }
  span<const T> underlying_buffer() const { return buffer_; }

  T* data() {
    // Allow accessing the position of `end()`.
    ONE_HARDENING_ASSERT(position_ <= buffer_.size());
    return buffer_.data() + position_;
  }

  const T* data() const {
    // Allow accessing the position of `end()`.
    ONE_HARDENING_ASSERT(position_ <= buffer_.size());
    return buffer_.data() + position_;
  }

  //
  // Position
  //

  [[nodiscard]] size_t position() const { return position_; }
  void set_position(size_t position) {
    ONE_HARDENING_ASSERT(position <= buffer_.size());
    position_ = position;
  }
  void Rewind() { position_ = 0; }

  //
  // Capacity
  //

  [[nodiscard]] bool empty() const { return size() == 0; }
  [[nodiscard]] size_t size() const { return position_; }
  [[nodiscard]] size_t max_size() const { return buffer_.max_size(); }
  [[nodiscard]] size_t remaining_size() const {
    return buffer_.size() - position_;
  }

  //
  // Bytes writing
  //

  template <typename U>
  void Write(span<const U> bytes) {
    static_assert(sizeof(U) == sizeof(T) && std::is_scalar<U>::value,
                  "U must be scalar type which size is same as one byte.");
    ONE_HARDENING_ASSERT(position_ + bytes.size() <= buffer_.size());

    ::memcpy(data(), bytes.data(), bytes.size());
    position_ += bytes.size();
  }

  //
  // Int16 & UInt16 writing
  //

  void Write(int16_t value) {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return WriteLe(value);
    }

    return WriteBe(value);
  }

  void WriteLe(int16_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::little_endian::Store16(data(), absl::bit_cast<uint16_t>(value));
    position_ += sizeof(value);
  }

  void WriteBe(int16_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::big_endian::Store16(data(), absl::bit_cast<uint16_t>(value));
    position_ += sizeof(value);
  }

  void Write(uint16_t value) {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return WriteLe(value);
    }

    return WriteBe(value);
  }

  void WriteLe(uint16_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::little_endian::Store16(data(), value);
    position_ += sizeof(value);
  }

  void WriteBe(uint16_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::big_endian::Store16(data(), value);
    position_ += sizeof(value);
  }

  //
  // Int32 & UInt32 writing
  //

  void Write(int32_t value) {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return WriteLe(value);
    }

    return WriteBe(value);
  }

  void WriteLe(int32_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::little_endian::Store32(data(), absl::bit_cast<uint32_t>(value));
    position_ += sizeof(value);
  }

  void WriteBe(int32_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::big_endian::Store32(data(), absl::bit_cast<uint32_t>(value));
    position_ += sizeof(value);
  }

  void Write(uint32_t value) {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return WriteLe(value);
    }

    return WriteBe(value);
  }

  void WriteLe(uint32_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::little_endian::Store32(data(), value);
    position_ += sizeof(value);
  }

  void WriteBe(uint32_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::big_endian::Store32(data(), value);
    position_ += sizeof(value);
  }

  //
  // Int64 & UInt64 writing
  //

  void Write(int64_t value) {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return WriteLe(value);
    }

    return WriteBe(value);
  }

  void WriteLe(int64_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::little_endian::Store64(data(), absl::bit_cast<uint64_t>(value));
    position_ += sizeof(value);
  }

  void WriteBe(int64_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::big_endian::Store64(data(), absl::bit_cast<uint64_t>(value));
    position_ += sizeof(value);
  }

  void Write(uint64_t value) {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return WriteLe(value);
    }

    return WriteBe(value);
  }

  void WriteLe(uint64_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::little_endian::Store64(data(), value);
    position_ += sizeof(value);
  }

  void WriteBe(uint64_t value) {
    ONE_HARDENING_ASSERT(position_ + sizeof(value) <= buffer_.size());

    absl::big_endian::Store64(data(), value);
    position_ += sizeof(value);
  }

  //
  // Varint writing
  //

  bool WriteVarint(int16_t value) {
    return WriteVarint(static_cast<uint64_t>(value));
  }

  bool WriteVarint(int32_t value) {
    return WriteVarint(static_cast<uint64_t>(value));
  }

  template <typename U>
  typename std::enable_if<std::is_unsigned<U>::value, bool>::type WriteVarint(
      U value) {
    constexpr std::byte kByteMostSignificantBitMask{0x80};
    constexpr size_t kByteWithoutMsbBitCount{7};

    if (remaining_size() < 1) {
      return false;
    }

    std::byte* ptr = data();

    ptr[0] = static_cast<std::byte>(value);
    position_++;
    if (value < std::to_integer<U>(kByteMostSignificantBitMask)) {
      return true;
    }

    if (remaining_size() < 1) {
      return false;
    }

    // Turn on continuation bit in the byte we just wrote.
    ptr[0] |= kByteMostSignificantBitMask;
    value >>= kByteWithoutMsbBitCount;
    ptr[1] = static_cast<std::byte>(value);
    position_++;
    if (value < std::to_integer<U>(kByteMostSignificantBitMask)) {
      return true;
    }

    ptr += 2;
    do {
      if (remaining_size() < 1) {
        return false;
      }

      // Turn on continuation bit in the byte we just wrote.
      ptr[-1] |= kByteMostSignificantBitMask;
      value >>= kByteWithoutMsbBitCount;
      *ptr = static_cast<std::byte>(value);
      ++ptr;
      position_++;
    } while (value >= std::to_integer<U>(kByteMostSignificantBitMask));

    return true;
  }

 private:
  span<T> buffer_;
  size_t position_{0};
};

//
// Helper method for creation with type deduction.
//

template <typename T>
BinaryWriter<T> MakeBinaryWriter(span<T> buffer) {
  return BinaryWriter<T>(buffer);
}

}  // namespace hcoona
