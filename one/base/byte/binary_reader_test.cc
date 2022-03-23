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

#include "one/base/byte/binary_reader.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>

#include "gtest/gtest.h"
#include "one/base/container/span.h"
#include "one/test/status.h"

//
// Varint tests vendor from Protobuf source code.
//
// Copied from
// https://github.com/protocolbuffers/protobuf/blob/7ecf43f0cedc4320c1cb31ba787161011b62e741/src/google/protobuf/io/coded_stream_unittest.cc
//
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
class BinaryReaderTest : public testing::TestWithParam<T> {
 protected:
  // Buffer used during most of the tests. This assumes tests run sequentially.
  static constexpr size_t kBufferSize = static_cast<size_t>(1024) * 64;
  static inline std::byte buffer_[kBufferSize];
};

// -------------------------------------------------------------------
// Varint tests.

struct VarintCase {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  uint8_t bytes[10];  // Encoded bytes.
  int size;           // Encoded size, in bytes.
  uint64_t value;     // Parsed value.
};

inline std::ostream& operator<<(std::ostream& os, const VarintCase& c) {
  return os << c.value;
}

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
VarintCase kVarintCases[] = {
    // 32-bit values
    {{0x00}, 1, 0},
    {{0x01}, 1, 1},
    {{0x7f}, 1, 127},
    {{0xa2, 0x74}, 2, (0x22 << 0) | (0x74 << 7)},  // 14882
    {{0xbe, 0xf7, 0x92, 0x84, 0x0b},
     5,  // 2961488830
     (0x3e << 0) | (0x77 << 7) | (0x12 << 14) | (0x04 << 21) |
         (uint64_t{0x0bu} << 28)},

    // 64-bit
    {{0xbe, 0xf7, 0x92, 0x84, 0x1b},
     5,  // 7256456126
     (0x3e << 0) | (0x77 << 7) | (0x12 << 14) | (0x04 << 21) |
         (uint64_t{0x1bu} << 28)},
    {{0x80, 0xe6, 0xeb, 0x9c, 0xc3, 0xc9, 0xa4, 0x49},
     8,  // 41256202580718336
     (0x00 << 0) | (0x66 << 7) | (0x6b << 14) | (0x1c << 21) |
         (uint64_t{0x43u} << 28) | (uint64_t{0x49u} << 35) |
         (uint64_t{0x24u} << 42) | (uint64_t{0x49u} << 49)},
    // 11964378330978735131
    {{0x9b, 0xa8, 0xf9, 0xc2, 0xbb, 0xd6, 0x80, 0x85, 0xa6, 0x01},
     10,
     (0x1b << 0) | (0x28 << 7) | (0x79 << 14) | (0x42 << 21) |
         (uint64_t{0x3bu} << 28) | (uint64_t{0x56u} << 35) |
         (uint64_t{0x00u} << 42) | (uint64_t{0x05u} << 49) |
         (uint64_t{0x26u} << 56) | (uint64_t{0x01u} << 63)},
};
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

using BinaryReaderSucceededTest = BinaryReaderTest<VarintCase>;

TEST_P(BinaryReaderSucceededTest, ReadVarint32) {
  const VarintCase& kVarintCases_case = GetParam();
  ::memcpy(buffer_, kVarintCases_case.bytes, kVarintCases_case.size);

  {
    auto reader = hcoona::MakeBinaryReader(
        hcoona::span<const std::byte>(buffer_, sizeof(buffer_)));

    std::optional<uint32_t> value = reader.ReadVarint32();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(static_cast<uint32_t>(kVarintCases_case.value), value);
  }
}

TEST_P(BinaryReaderSucceededTest, ReadVarint64) {
  const VarintCase& kVarintCases_case = GetParam();
  ::memcpy(buffer_, kVarintCases_case.bytes, kVarintCases_case.size);

  {
    auto reader = hcoona::MakeBinaryReader(
        hcoona::span<const std::byte>(buffer_, sizeof(buffer_)));

    std::optional<uint64_t> value = reader.ReadVarint64();
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(kVarintCases_case.value, value);
  }
}

INSTANTIATE_TEST_SUITE_P(VarintSucceededCases, BinaryReaderSucceededTest,
                         ::testing::ValuesIn(kVarintCases));

// -------------------------------------------------------------------
// Varint failure test.

struct VarintErrorCase {
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
  uint8_t bytes[12];
  int size;
  bool can_parse;
};

inline std::ostream& operator<<(std::ostream& os, const VarintErrorCase& c) {
  return os << "size " << c.size;
}

const VarintErrorCase kVarintErrorCases[] = {
    // Control case.  (Insures that there isn't something else wrong that
    // makes parsing always fail.)
    {{0x00}, 1, true},

    // No input data.
    {{}, 0, false},

    // Input ends unexpectedly.
    {{0xf0, 0xab}, 2, false},

    // Input ends unexpectedly after 32 bits.
    {{0xf0, 0xab, 0xc9, 0x9a, 0xf8, 0xb2}, 6, false},

    // Longer than 10 bytes.
    {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01},
     11,
     false},
};

using BinaryReaderFailureTest = BinaryReaderTest<VarintErrorCase>;

TEST_P(BinaryReaderFailureTest, ReadVarint32Error) {
  const auto& kVarintErrorCases_case = GetParam();
  ::memcpy(buffer_, kVarintErrorCases_case.bytes, kVarintErrorCases_case.size);

  auto reader = hcoona::MakeBinaryReader(
      hcoona::span<const std::byte>(buffer_, kVarintErrorCases_case.size));

  std::optional<uint32_t> value = reader.ReadVarint32();
  EXPECT_EQ(kVarintErrorCases_case.can_parse, value.has_value())
      << (value.has_value() ? value.value() : -1);
}

TEST_P(BinaryReaderFailureTest, ReadVarint64Error) {
  const auto& kVarintErrorCases_case = GetParam();
  ::memcpy(buffer_, kVarintErrorCases_case.bytes, kVarintErrorCases_case.size);

  auto reader = hcoona::MakeBinaryReader(
      hcoona::span<const std::byte>(buffer_, kVarintErrorCases_case.size));

  std::optional<uint64_t> value = reader.ReadVarint64();
  EXPECT_EQ(kVarintErrorCases_case.can_parse, value.has_value());
}

INSTANTIATE_TEST_SUITE_P(VarintFailuresCases, BinaryReaderFailureTest,
                         ::testing::ValuesIn(kVarintErrorCases));

//
// Continuation reading test.
//

TEST(BinaryReaderContinuesReadingTest, ReadInt16Int32) {
  static const uint8_t kBytes[]{0x00, 0x80, 0x00, 0x00, 0x00, 0x01};
  auto reader = hcoona::MakeBinaryReader(hcoona::span<const uint8_t>(kBytes));

  EXPECT_EQ(static_cast<int16_t>(0x8000), reader.ReadInt16());
  EXPECT_EQ(static_cast<int32_t>(0x01000000), reader.ReadInt32());
  EXPECT_TRUE(reader.empty());
}

TEST(BinaryReaderContinuesReadingTest, ReadVarint16Varint32) {
  static const uint8_t kBytes[]{0x01, 0x96, 0x01};
  auto reader = hcoona::MakeBinaryReader(hcoona::span<const uint8_t>(kBytes));

  EXPECT_EQ(static_cast<uint32_t>(0x01), reader.ReadVarint32());
  EXPECT_EQ(static_cast<uint32_t>(150), reader.ReadVarint32());
  EXPECT_TRUE(reader.empty());
}
