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

#include "one/codelab/minikafka/kafka_binary_reader.h"

#include <utility>

#include "glog/logging.h"

namespace hcoona {
namespace minikafka {

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

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

namespace {

constexpr int kMaxVarintBytes = 10;
constexpr int kMaxVarint32Bytes = 5;

// Decodes varint64 with known size, N, and returns next pointer. Knowing N at
// compile time, compiler can generate optimal code. For example, instead of
// subtracting 0x80 at each iteration, it subtracts properly shifted mask once.
template <size_t N>
const uint8_t* DecodeVarint64KnownSize(const uint8_t* buffer, uint64_t* value) {
  DCHECK_GT(N, 0U);
  uint64_t result = static_cast<uint64_t>(buffer[N - 1]) << (7U * (N - 1));
  for (size_t i = 0, offset = 0; i < N - 1; i++, offset += 7) {
    result += static_cast<uint64_t>(buffer[i] - 0x80U) << offset;
  }
  *value = result;
  return buffer + N;
}

// Read a varint from the given buffer, write it to *value, and return a pair.
// The first part of the pair is true iff the read was successful.  The second
// part is buffer + (number of bytes read).  This function is always inlined,
// so returning a pair is costless.
inline ::std::pair<bool, const uint8_t*> ReadVarint32FromArray(
    uint32_t first_byte, const uint8_t* buffer, uint32_t* value) {
  // Fast path:  We have enough bytes left in the buffer to guarantee that
  // this read won't cross the end, so we can skip the checks.
  DCHECK_EQ(*buffer, first_byte);
  DCHECK_EQ(first_byte & 0x80U, 0x80U) << first_byte;
  const uint8_t* ptr = buffer;
  uint32_t b;
  uint32_t result = first_byte - 0x80U;
  ++ptr;  // We just processed the first byte.  Move on to the second.
  b = *(ptr++);
  result += b << 7U;
  if ((b & 0x80U) == 0U) {
    goto done;
  }
  result -= 0x80U << 7U;
  b = *(ptr++);
  result += b << 14U;
  if ((b & 0x80U) == 0U) {
    goto done;
  }
  result -= 0x80U << 14U;
  b = *(ptr++);
  result += b << 21U;
  if ((b & 0x80U) == 0U) {
    goto done;
  }
  result -= 0x80U << 21U;
  b = *(ptr++);
  result += b << 28U;
  if ((b & 0x80U) == 0U) {
    goto done;
  }
  // "result -= 0x80U << 28" is irrevelant.

  // If the input is larger than 32 bits, we still need to read it all
  // and discard the high-order bits.
  for (int i = 0; i < kMaxVarintBytes - kMaxVarint32Bytes; i++) {
    b = *(ptr++);
    if ((b & 0x80U) == 0U) {
      goto done;
    }
  }

  // We have overrun the maximum size of a varint (10 bytes).  Assume
  // the data is corrupt.
  return std::make_pair(false, ptr);

done:
  *value = result;
  return std::make_pair(true, ptr);
}

inline ::std::pair<bool, const uint8_t*> ReadVarint64FromArray(
    const uint8_t* buffer, uint64_t* value) {
  // Assumes varint64 is at least 2 bytes.
  DCHECK_GE(buffer[0], 128);

  const uint8_t* next;
  if (buffer[1] < 128) {
    next = DecodeVarint64KnownSize<2>(buffer, value);
  } else if (buffer[2] < 128) {
    next = DecodeVarint64KnownSize<3>(buffer, value);
  } else if (buffer[3] < 128) {
    next = DecodeVarint64KnownSize<4>(buffer, value);
  } else if (buffer[4] < 128) {
    next = DecodeVarint64KnownSize<5>(buffer, value);
  } else if (buffer[5] < 128) {
    next = DecodeVarint64KnownSize<6>(buffer, value);
  } else if (buffer[6] < 128) {
    next = DecodeVarint64KnownSize<7>(buffer, value);
  } else if (buffer[7] < 128) {
    next = DecodeVarint64KnownSize<8>(buffer, value);
  } else if (buffer[8] < 128) {
    next = DecodeVarint64KnownSize<9>(buffer, value);
  } else if (buffer[9] < 128) {
    next = DecodeVarint64KnownSize<10>(buffer, value);
  } else {
    // We have overrun the maximum size of a varint (10 bytes). Assume
    // the data is corrupt.
    return std::make_pair(false, buffer + 11);
  }

  return std::make_pair(true, next);
}

}  // namespace

bool KafkaBinaryReader::ReadVarint32(std::uint32_t* value) {
  uint32_t v = 0;
  if (ABSL_PREDICT_TRUE(current_ < end_)) {
    v = *current_;
    if (v < 0x80U) {
      *value = v;
      current_++;
      return true;
    }
  }
  int64_t result = ReadVarint32Fallback(v);
  *value = static_cast<uint32_t>(result);
  return result >= 0;
}

bool KafkaBinaryReader::ReadVarint64(std::uint64_t* value) {
  if (ABSL_PREDICT_TRUE(current_ < end_) && *current_ < 0x80U) {
    *value = *current_;
    current_++;
    return true;
  }
  std::pair<uint64_t, bool> p = ReadVarint64Fallback();
  *value = p.first;
  return p.second;
}

bool KafkaBinaryReader::ReadVarint32Slow(uint32_t* value) {
  // Directly invoke ReadVarint64Fallback, since we already tried to optimize
  // for one-byte varints.
  std::pair<uint64_t, bool> p = ReadVarint64Fallback();
  *value = static_cast<uint32_t>(p.first);
  return p.second;
}

int64_t KafkaBinaryReader::ReadVarint32Fallback(uint32_t first_byte_or_zero) {
  if (end_ - current_ >= kMaxVarintBytes ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (end_ > current_ && !(end_[-1] & 0x80U))) {
    DCHECK_NE(first_byte_or_zero, 0U)
        << "Caller should provide us with *buffer_ when buffer is non-empty";
    uint32_t temp;
    ::std::pair<bool, const uint8_t*> p =
        ReadVarint32FromArray(first_byte_or_zero, current_, &temp);
    if (!p.first) {
      return -1;
    }
    current_ = p.second;
    return temp;
  }

  // Really slow case: we will incur the cost of an extra function call here,
  // but moving this out of line reduces the size of this function, which
  // improves the common case. In micro benchmarks, this is worth about 10-15%
  uint32_t temp;
  return ReadVarint32Slow(&temp) ? static_cast<int64_t>(temp) : -1;
}

bool KafkaBinaryReader::ReadVarint64Slow(uint64_t* value) {
  // Slow path:  This read might cross the end of the buffer, so we
  // need to check and refresh the buffer if and when it does.

  uint64_t result = 0;
  int count = 0;
  uint32_t b;

  do {
    if (count == kMaxVarintBytes) {
      *value = 0;
      return false;
    }
    while (current_ == end_) {
      *value = 0;
      return false;
    }
    b = *current_;
    result |= static_cast<uint64_t>(b & 0x7FU) << (7U * count);
    current_++;
    ++count;
  } while ((b & 0x80U) != 0U);

  *value = result;
  return true;
}

std::pair<uint64_t, bool> KafkaBinaryReader::ReadVarint64Fallback() {
  if (end_ - current_ >= kMaxVarintBytes ||
      // Optimization:  We're also safe if the buffer is non-empty and it ends
      // with a byte that would terminate a varint.
      (end_ > current_ && !(end_[-1] & 0x80U))) {
    uint64_t temp;
    ::std::pair<bool, const uint8_t*> p =
        ReadVarint64FromArray(current_, &temp);
    if (!p.first) {
      return std::make_pair(0, false);
    }
    current_ = p.second;
    return std::make_pair(temp, true);
  }
  uint64_t temp;
  bool success = ReadVarint64Slow(&temp);
  return std::make_pair(temp, success);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

}  // namespace minikafka
}  // namespace hcoona
