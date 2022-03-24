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
// File: binary_reader.h
// -----------------------------------------------------------------------------
//
// This header file defines a reader class help parse data from underlying bytes
// span.

#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

#include "absl/base/internal/endian.h"
#include "one/base/container/span.h"
#include "one/base/macros.h"
#include "one/base/narrow_cast.h"

namespace hcoona {

template <typename T>
class BinaryReader {
  static_assert(sizeof(T) == 1 && std::is_scalar<T>::value,
                "T must be a scalar type which size is same as a byte.");

 public:
  explicit BinaryReader(span<const T> buffer)
      : begin_(buffer.data()),
        current_(buffer.data()),
        end_(buffer.data() + buffer.size()) {}

  // Implicit copy & move are allowed.

  //
  // Element access
  //

  span<const T> underlying_buffer() const { return {begin_, end_}; }

  const T* data() const {
    // Allow accessing the position of `end()`.
    ONE_HARDENING_ASSERT(current_ <= end_);
    return current_;
  }

  //
  // Position
  //

  [[nodiscard]] size_t position() const { return current_ - begin_; }
  void set_position(size_t position) {
    ONE_HARDENING_ASSERT(begin_ + position <= end_);
    current_ = begin_ + position;
  }
  void Rewind() { current_ = begin_; }
  void Advance(size_t n) {
    ONE_HARDENING_ASSERT(current_ + n <= end_);
    current_ = current_ + n;
  }

  //
  // Capacity
  //

  [[nodiscard]] bool empty() const { return size() == 0; }
  [[nodiscard]] size_t size() const { return end_ - current_; }
  [[nodiscard]] size_t max_size() const { return end_ - begin_; }

  //
  // Bytes reading
  //

  [[nodiscard]] span<const T> Peek(size_t n = 1) const {
    ONE_HARDENING_ASSERT(current_ + n <= end_);
    return {current_, n};
  }

  [[nodiscard]] span<const T> Read(size_t n = 1) {
    ONE_HARDENING_ASSERT(current_ + n <= end_);
    span<const T> result = {current_, n};
    current_ = current_ + n;
    return result;
  }

  //
  // Int16 & UInt16 reading
  //

  [[nodiscard]] uint16_t PeekUInt16() const {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return PeekUInt16Le();
    }
    return PeekUInt16Be();
  }

  [[nodiscard]] uint16_t PeekUInt16Le() const {
    ONE_HARDENING_ASSERT(current_ + sizeof(uint16_t) <= end_);
    return absl::little_endian::Load16(Peek(sizeof(uint16_t)).data());
  }

  [[nodiscard]] uint16_t PeekUInt16Be() const {
    ONE_HARDENING_ASSERT(current_ + sizeof(uint16_t) <= end_);
    return absl::big_endian::Load16(Peek(sizeof(uint16_t)).data());
  }

  [[nodiscard]] int16_t PeekInt16() const {
    return absl::bit_cast<int16_t>(PeekUInt16());
  }

  [[nodiscard]] uint16_t PeekInt16Le() const {
    return absl::bit_cast<int16_t>(PeekUInt16Le());
  }

  [[nodiscard]] uint16_t PeekInt16Be() const {
    return absl::bit_cast<int16_t>(PeekUInt16Be());
  }

  [[nodiscard]] uint16_t ReadUInt16() {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return ReadUInt16Le();
    }
    return ReadUInt16Be();
  }

  [[nodiscard]] uint16_t ReadUInt16Le() {
    uint16_t result = PeekUInt16Le();
    current_ += sizeof(uint16_t);
    return result;
  }

  [[nodiscard]] uint16_t ReadUInt16Be() {
    uint16_t result = PeekUInt16Be();
    current_ += sizeof(uint16_t);
    return result;
  }

  [[nodiscard]] int16_t ReadInt16() {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return ReadInt16Le();
    }
    return ReadInt16Be();
  }

  [[nodiscard]] int16_t ReadInt16Le() {
    int16_t result = PeekInt16Le();
    current_ += sizeof(int16_t);
    return result;
  }

  [[nodiscard]] int16_t ReadInt16Be() {
    int16_t result = PeekInt16Be();
    current_ += sizeof(int16_t);
    return result;
  }

  //
  // Int32 & UInt32 reading
  //

  [[nodiscard]] uint32_t PeekUInt32() const {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return PeekUInt32Le();
    }
    return PeekUInt32Be();
  }

  [[nodiscard]] uint32_t PeekUInt32Le() const {
    ONE_HARDENING_ASSERT(current_ + sizeof(uint32_t) <= end_);
    return absl::little_endian::Load32(Peek(sizeof(uint32_t)).data());
  }

  [[nodiscard]] uint32_t PeekUInt32Be() const {
    ONE_HARDENING_ASSERT(current_ + sizeof(uint32_t) <= end_);
    return absl::big_endian::Load32(Peek(sizeof(uint32_t)).data());
  }

  [[nodiscard]] int32_t PeekInt32() const {
    return absl::bit_cast<int32_t>(PeekUInt32());
  }

  [[nodiscard]] uint32_t PeekInt32Le() const {
    return absl::bit_cast<int32_t>(PeekUInt32Le());
  }

  [[nodiscard]] uint32_t PeekInt32Be() const {
    return absl::bit_cast<int32_t>(PeekUInt32Be());
  }

  [[nodiscard]] uint32_t ReadUInt32() {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return ReadUInt32Le();
    }
    return ReadUInt32Be();
  }

  [[nodiscard]] uint32_t ReadUInt32Le() {
    uint32_t result = PeekUInt32Le();
    current_ += sizeof(uint32_t);
    return result;
  }

  [[nodiscard]] uint32_t ReadUInt32Be() {
    uint32_t result = PeekUInt32Be();
    current_ += sizeof(uint32_t);
    return result;
  }

  [[nodiscard]] int32_t ReadInt32() {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return ReadInt32Le();
    }
    return ReadInt32Be();
  }

  [[nodiscard]] int32_t ReadInt32Le() {
    int32_t result = PeekInt32Le();
    current_ += sizeof(int32_t);
    return result;
  }

  [[nodiscard]] int32_t ReadInt32Be() {
    int32_t result = PeekInt32Be();
    current_ += sizeof(int32_t);
    return result;
  }

  //
  // Int64 & UInt64 reading
  //

  [[nodiscard]] uint64_t PeekUInt64() const {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return PeekUInt64Le();
    }
    return PeekUInt64Be();
  }

  [[nodiscard]] uint64_t PeekUInt64Le() const {
    ONE_HARDENING_ASSERT(current_ + sizeof(uint64_t) <= end_);
    return absl::little_endian::Load64(Peek(sizeof(uint64_t)).data());
  }

  [[nodiscard]] uint64_t PeekUInt64Be() const {
    ONE_HARDENING_ASSERT(current_ + sizeof(uint64_t) <= end_);
    return absl::big_endian::Load64(Peek(sizeof(uint64_t)).data());
  }

  [[nodiscard]] int64_t PeekInt64() const {
    return absl::bit_cast<int64_t>(PeekUInt64());
  }

  [[nodiscard]] uint64_t PeekInt64Le() const {
    return absl::bit_cast<int64_t>(PeekUInt64Le());
  }

  [[nodiscard]] uint64_t PeekInt64Be() const {
    return absl::bit_cast<int64_t>(PeekUInt64Be());
  }

  [[nodiscard]] uint64_t ReadUInt64() {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return ReadUInt64Le();
    }
    return ReadUInt64Be();
  }

  [[nodiscard]] uint64_t ReadUInt64Le() {
    uint64_t result = PeekUInt64Le();
    current_ += sizeof(uint64_t);
    return result;
  }

  [[nodiscard]] uint64_t ReadUInt64Be() {
    uint64_t result = PeekUInt64Be();
    current_ += sizeof(uint64_t);
    return result;
  }

  [[nodiscard]] int64_t ReadInt64() {
    if constexpr (absl::little_endian::IsLittleEndian()) {
      return ReadInt64Le();
    }
    return ReadInt64Be();
  }

  [[nodiscard]] int64_t ReadInt64Le() {
    int64_t result = PeekInt64Le();
    current_ += sizeof(int64_t);
    return result;
  }

  [[nodiscard]] int64_t ReadInt64Be() {
    int64_t result = PeekInt64Be();
    current_ += sizeof(int64_t);
    return result;
  }

  //
  // Varint reading
  //

  std::optional<uint32_t> ReadVarint32() {
    // We need at least 1 byte.
    if (empty()) {
      return std::nullopt;
    }

    std::byte v = *as_bytes(Peek()).data();
    if (v < kByteMostSignificantBitMask) {
      Advance(1);
      return std::to_integer<uint32_t>(v);
    }

    int64_t value = ReadVarint32Fallback(v);
    if (value < 0) {
      return std::nullopt;
    }

    return narrow_cast<uint32_t>(value);
  }

  std::optional<uint64_t> ReadVarint64() {
    // We need at least 1 byte.
    if (empty()) {
      return std::nullopt;
    }

    std::byte v = *as_bytes(Peek()).data();
    if (v < kByteMostSignificantBitMask) {
      Advance(1);
      return std::to_integer<uint64_t>(v);
    }

    return ReadVarint64Fallback();
  }

 private:
  static constexpr std::byte kByteMostSignificantBitMask{0x80};

  // Return negative value for error. If the returned value is non-negative,
  // it's garanteed the value could be fit into uint32_t.
  int64_t ReadVarint32Fallback(std::byte first_byte);
  std::optional<uint64_t> ReadVarint64Fallback();

  int64_t ReadVarint32Slow() {
    // Directly invoke ReadVarint64Fallback, since we already tried to optimize
    // for one-byte varints.
    std::optional<uint64_t> value = ReadVarint64Fallback();
    return value.has_value()
               ? static_cast<int64_t>(narrow_cast<uint32_t>(value.value()))
               : -1;
  }
  std::optional<uint64_t> ReadVarint64Slow();

  const T* begin_;
  const T* current_;
  const T* end_;
};

//
// Vendor reading varint methods from protobuf source code.
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
//
// Modifications:
// 1. Use `std::byte` for byte.
// 2. Adapt buffer size checking.
// 3. Use our hardening macro.
// 4. Use our constants.
// 5. Use `std::optional<T>` instead of `pair<bool, T>`.
// 6. Put non-member functions into `details` namespace.
// 7. Don't use `goto` statement.

namespace details {

constexpr std::byte kByteMostSignificantBitMask{0x80};
constexpr std::byte kByteWithoutMostSignificantBitMask{0x7F};

constexpr size_t kByteWithoutMsbBitCount{7};
constexpr size_t kVarintBytesMax{10};
constexpr size_t kVarint32BytesMax{5};

// Read a varint from the given buffer, write it to *value, and return a pair.
// The first part of the pair is true iff the read was successful.  The second
// part is buffer + (number of bytes read).  This function is always inlined,
// so returning a pair is costless.
template <typename T>
inline ::std::optional<uint32_t> ReadVarint32FromArray(std::byte first_byte,
                                                       const T* buffer,
                                                       const T** new_buffer) {
  static constexpr uint32_t kByteMostSignificantBitMaskUInt32{
      std::to_integer<uint32_t>(kByteMostSignificantBitMask)};

  // Fast path:  We have enough bytes left in the buffer to guarantee that
  // this read won't cross the end, so we can skip the checks.
  ONE_ASSERT(std::byte(*buffer) == first_byte);
  ONE_ASSERT((first_byte & kByteMostSignificantBitMask) ==
             kByteMostSignificantBitMask);

  auto result = std::to_integer<uint32_t>(first_byte &
                                          kByteWithoutMostSignificantBitMask);

  const T* ptr = buffer;
  ++ptr;  // We just processed the first byte.  Move on to the second.

  auto b = static_cast<uint32_t>(*(ptr++));
  result += (b << kByteWithoutMsbBitCount);
  if ((b & kByteMostSignificantBitMaskUInt32) == 0U) {
    *new_buffer = ptr;
    return result;
  }
  result -= (kByteMostSignificantBitMaskUInt32 << kByteWithoutMsbBitCount);

  b = static_cast<uint32_t>(*(ptr++));
  result += (b << (kByteWithoutMsbBitCount * 2));
  if ((b & kByteMostSignificantBitMaskUInt32) == 0U) {
    *new_buffer = ptr;
    return result;
  }
  result -=
      (kByteMostSignificantBitMaskUInt32 << (kByteWithoutMsbBitCount * 2));

  b = static_cast<uint32_t>(*(ptr++));
  result += (b << (kByteWithoutMsbBitCount * 3));
  if ((b & kByteMostSignificantBitMaskUInt32) == 0U) {
    *new_buffer = ptr;
    return result;
  }
  result -=
      (kByteMostSignificantBitMaskUInt32 << (kByteWithoutMsbBitCount * 3));

  b = static_cast<uint32_t>(*(ptr++));
  result += (b << (kByteWithoutMsbBitCount * 4));
  if ((b & kByteMostSignificantBitMaskUInt32) == 0U) {
    *new_buffer = ptr;
    return result;
  }
  // "result -= kByteMostSignificantBitMask << 28" is irrelevant.

  // If the input is larger than 32 bits, we still need to read it all
  // and discard the high-order bits.
  for (size_t i = 0; i < (kVarintBytesMax - kVarint32BytesMax); i++) {
    b = static_cast<uint32_t>(*(ptr++));
    if ((b & kByteMostSignificantBitMaskUInt32) == 0U) {
      *new_buffer = ptr;
      return result;
    }
  }

  // We have overrun the maximum size of a varint (10 bytes).  Assume
  // the data is corrupt.
  *new_buffer = ptr;
  return std::nullopt;
}

// Decodes varint64 with known size, N, and returns next pointer. Knowing N at
// compile time, compiler can generate optimal code. For example, instead of
// subtracting 0x80 at each iteration, it subtracts properly shifted mask once.
template <typename T, size_t N>
inline uint64_t DecodeVarint64KnownSize(const T* buffer, const T** new_buffer) {
  static_assert(N > 0, "Cannot be empty.");

  uint64_t result = static_cast<uint64_t>(buffer[N - 1])
                    << (kByteWithoutMsbBitCount * (N - 1));
  for (size_t i = 0, offset = 0; i < N - 1;
       i++, offset += kByteWithoutMsbBitCount) {
    result += (static_cast<uint64_t>(buffer[i]) -
               static_cast<uint64_t>(kByteMostSignificantBitMask))
              << offset;
  }

  *new_buffer = buffer + N;
  return result;
}

template <typename T>
inline std::optional<uint64_t> ReadVarint64FromArray(const T* buffer,
                                                     const T** new_buffer) {
  // Assumes varint64 is at least 2 bytes.
  ONE_ASSERT(std::byte(buffer[0]) >= kByteMostSignificantBitMask);

  if (std::byte(buffer[1]) < kByteMostSignificantBitMask) {
    return DecodeVarint64KnownSize<T, 2>(buffer, new_buffer);
  }
  if (std::byte(buffer[2]) < kByteMostSignificantBitMask) {
    return DecodeVarint64KnownSize<T, 3>(buffer, new_buffer);
  }
  if (std::byte(buffer[3]) < kByteMostSignificantBitMask) {
    return DecodeVarint64KnownSize<T, 4>(buffer, new_buffer);
  }
  if (std::byte(buffer[4]) < kByteMostSignificantBitMask) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return DecodeVarint64KnownSize<T, 5>(buffer, new_buffer);
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  if (std::byte(buffer[5]) < kByteMostSignificantBitMask) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return DecodeVarint64KnownSize<T, 6>(buffer, new_buffer);
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  if (std::byte(buffer[6]) < kByteMostSignificantBitMask) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return DecodeVarint64KnownSize<T, 7>(buffer, new_buffer);
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  if (std::byte(buffer[7]) < kByteMostSignificantBitMask) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return DecodeVarint64KnownSize<T, 8>(buffer, new_buffer);
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  if (std::byte(buffer[8]) < kByteMostSignificantBitMask) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return DecodeVarint64KnownSize<T, 9>(buffer, new_buffer);
  }
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  if (std::byte(buffer[9]) < kByteMostSignificantBitMask) {
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    return DecodeVarint64KnownSize<T, 10>(buffer, new_buffer);
  }

  // We have overrun the maximum size of a varint (10 bytes). Assume
  // the data is corrupt.
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  *new_buffer = buffer + 11;
  return std::nullopt;
}

}  // namespace details

template <typename T>
int64_t BinaryReader<T>::ReadVarint32Fallback(std::byte first_byte_or_zero) {
  if (size() >= details::kVarintBytesMax ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (!empty() &&
       std::to_integer<uint8_t>(std::byte(end_[-1]) &
                                kByteMostSignificantBitMask) == 0)) {
    // Caller should provide us with *buffer_ when buffer is non-empty
    ONE_ASSERT(first_byte_or_zero != std::byte(0));
    const T* new_buffer_begin;
    std::optional<uint32_t> result = details::ReadVarint32FromArray(
        first_byte_or_zero, data(), &new_buffer_begin);
    if (!result) {
      return -1;
    }
    current_ = new_buffer_begin;
    return static_cast<int64_t>(result.value());
  }

  // Really slow case: we will incur the cost of an extra function call here,
  // but moving this out of line reduces the size of this function, which
  // improves the common case. In micro benchmarks, this is worth about 10-15%
  return ReadVarint32Slow();
}

template <typename T>
std::optional<uint64_t> BinaryReader<T>::ReadVarint64Fallback() {
  if (size() >= details::kVarintBytesMax ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (!empty() &&
       std::to_integer<uint8_t>(std::byte(end_[-1]) &
                                kByteMostSignificantBitMask) == 0)) {
    const T* new_buffer_begin;
    std::optional<uint64_t> result =
        details::ReadVarint64FromArray(current_, &new_buffer_begin);
    if (!result) {
      return std::nullopt;
    }
    current_ = new_buffer_begin;
    return result;
  }

  return ReadVarint64Slow();
}

template <typename T>
std::optional<uint64_t> BinaryReader<T>::ReadVarint64Slow() {
  // Slow path:  This read might cross the end of the buffer, so we
  // need to check and refresh the buffer if and when it does.

  uint64_t result = 0;
  size_t count = 0;
  std::byte b;

  do {
    if (count == details::kVarintBytesMax) {
      return std::nullopt;
    }
    if (empty()) {
      return std::nullopt;
    }

    b = std::byte(*current_);
    result |= std::to_integer<uint64_t>(
                  b & details::kByteWithoutMostSignificantBitMask)
              << (details::kByteWithoutMsbBitCount * count);
    Advance(1);
    ++count;
  } while (std::to_integer<uint8_t>(b & details::kByteMostSignificantBitMask) !=
           0);

  return result;
}

//
// Helper method for creation with type deduction.
//

template <typename T>
BinaryReader<T> MakeBinaryReader(span<const T> buffer) {
  return BinaryReader<T>(buffer);
}

}  // namespace hcoona
