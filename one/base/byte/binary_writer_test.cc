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

#include "one/base/byte/binary_writer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>

#include "gtest/gtest.h"
#include "one/base/byte/binary_reader.h"
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
class BinaryWriterTest : public testing::TestWithParam<T> {
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

using BinaryWriterSucceededTest = BinaryWriterTest<VarintCase>;

TEST_P(BinaryWriterSucceededTest, WriteVarint32) {
  const auto& kVarintCases_case = GetParam();
  if (kVarintCases_case.value > uint64_t{0x00000000FFFFFFFFu}) {
    // Skip this test for the 64-bit values.
    return;
  }

  hcoona::BinaryWriter<std::byte> writer{hcoona::span<std::byte>(buffer_)};
  {
    ASSERT_TRUE(
        writer.WriteVarint(static_cast<uint32_t>(kVarintCases_case.value)));

    EXPECT_EQ(kVarintCases_case.size, writer.size());
  }

  EXPECT_EQ(0,
            memcmp(buffer_, kVarintCases_case.bytes, kVarintCases_case.size));
}

TEST_P(BinaryWriterSucceededTest, WriteVarint64) {
  const auto& kVarintCases_case = GetParam();

  hcoona::BinaryWriter<std::byte> writer{hcoona::span<std::byte>(buffer_)};
  {
    ASSERT_TRUE(writer.WriteVarint(kVarintCases_case.value));

    EXPECT_EQ(kVarintCases_case.size, writer.size());
  }

  EXPECT_EQ(0,
            memcmp(buffer_, kVarintCases_case.bytes, kVarintCases_case.size));
}

INSTANTIATE_TEST_SUITE_P(VarintSucceededCases, BinaryWriterSucceededTest,
                         ::testing::ValuesIn(kVarintCases));

int32_t kSignExtendedVarintCases[] = {0, 1, -1, 1237894, -37895138};

using BinaryWriterSignedExtendedSucceededTest = BinaryWriterTest<int32_t>;

TEST_P(BinaryWriterSignedExtendedSucceededTest, WriteVarint32SignExtended) {
  const auto& kSignExtendedVarintCases_case = GetParam();

  hcoona::BinaryWriter<std::byte> writer{hcoona::span<std::byte>(buffer_)};
  {
    ASSERT_TRUE(writer.WriteVarint(kSignExtendedVarintCases_case));

    if (kSignExtendedVarintCases_case < 0) {
      EXPECT_EQ(10, writer.size());
    } else {
      EXPECT_LE(writer.size(), 5);
    }
  }

  // Read value back in as a varint64 and insure it matches.
  hcoona::BinaryReader<std::byte> reader{hcoona::span<std::byte>(buffer_)};
  {
    std::optional<uint64_t> value = reader.ReadVarint64();
    ASSERT_TRUE(value.has_value());

    EXPECT_EQ(kSignExtendedVarintCases_case,
              static_cast<int64_t>(value.value()));
  }

  EXPECT_EQ(writer.size(), reader.position());
}

INSTANTIATE_TEST_SUITE_P(VarintSucceededCases,
                         BinaryWriterSignedExtendedSucceededTest,
                         ::testing::ValuesIn(kSignExtendedVarintCases));

//
// Continuation writing test.
//

TEST(BinaryWriterContinuesReadingTest, WriteNativeVarintBe) {
  static constexpr size_t kBufferSize{65535};
  static constexpr uint8_t kPoisonByte{0xFE};
  std::byte buffer[kBufferSize];
  ::memset(buffer, kPoisonByte, kBufferSize);

  auto reader = hcoona::MakeBinaryReader(hcoona::span<const std::byte>(buffer));
  auto writer = hcoona::MakeBinaryWriter(hcoona::span<std::byte>(buffer));

  writer.Write(static_cast<int16_t>(128));
  ASSERT_TRUE(writer.WriteVarint(static_cast<int32_t>(25500)));
  writer.WriteBe(static_cast<int32_t>(3555));

  EXPECT_EQ(static_cast<int32_t>(128), reader.ReadInt16());
  EXPECT_EQ(static_cast<int32_t>(25500), reader.ReadVarint32());
  EXPECT_EQ(static_cast<int32_t>(3555), reader.ReadInt32Be());
  EXPECT_EQ(reader.position(), writer.size());
}

TEST(BinaryWriterContinuesReadingTest, WriteVarint32Varint64Varint32) {
  static constexpr size_t kBufferSize{65535};
  static constexpr uint8_t kPoisonByte{0xFE};
  std::byte buffer[kBufferSize];
  ::memset(buffer, kPoisonByte, kBufferSize);

  auto reader = hcoona::MakeBinaryReader(hcoona::span<const std::byte>(buffer));
  auto writer = hcoona::MakeBinaryWriter(hcoona::span<std::byte>(buffer));

  ASSERT_TRUE(writer.WriteVarint(static_cast<uint32_t>(1U)));
  ASSERT_TRUE(writer.WriteVarint(static_cast<uint64_t>(2147483652ULL)));
  ASSERT_TRUE(writer.WriteVarint(static_cast<uint32_t>(2550U)));

  EXPECT_EQ(static_cast<uint32_t>(1U), reader.ReadVarint32());
  EXPECT_EQ(static_cast<uint64_t>(2147483652ULL), reader.ReadVarint64());
  EXPECT_EQ(static_cast<uint32_t>(2550U), reader.ReadVarint32());
  EXPECT_EQ(reader.position(), writer.size());
}
