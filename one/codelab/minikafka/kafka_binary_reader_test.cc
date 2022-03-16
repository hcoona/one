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

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "absl/base/macros.h"
#include "absl/types/span.h"
#include "gtest/gtest.h"
#include "one/test/macros.h"

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

// ===================================================================
// Data-Driven Test Infrastructure

// TEST_1D and TEST_2D are macros I'd eventually like to see added to
// gTest.  These macros can be used to declare tests which should be
// run multiple times, once for each item in some input array.  TEST_1D
// tests all cases in a single input array.  TEST_2D tests all
// combinations of cases from two arrays.  The arrays must be statically
// defined such that the GOOGLE_ARRAYSIZE() macro works on them.  Example:
//
// int kCases[] = {1, 2, 3, 4}
// TEST_1D(MyFixture, MyTest, kCases) {
//   EXPECT_GT(kCases_case, 0);
// }
//
// This test iterates through the numbers 1, 2, 3, and 4 and tests that
// they are all grater than zero.  In case of failure, the exact case
// which failed will be printed.  The case type must be printable using
// ostream::operator<<.

// TODO(kenton):  gTest now supports "parameterized tests" which would be
//   a better way to accomplish this.  Rewrite when time permits.

#define TEST_1D(FIXTURE, NAME, CASES)                             \
  class FIXTURE##_##NAME##_DD : public FIXTURE {                  \
   protected:                                                     \
    template <typename CaseType>                                  \
    void DoSingleCase(const CaseType& CASES##_case);              \
  };                                                              \
                                                                  \
  TEST_F(FIXTURE##_##NAME##_DD, NAME) {                           \
    for (size_t i = 0; i < ABSL_ARRAYSIZE(CASES); i++) {          \
      SCOPED_TRACE(testing::Message()                             \
                   << #CASES " case #" << i << ": " << CASES[i]); \
      DoSingleCase(CASES[i]);                                     \
    }                                                             \
  }                                                               \
                                                                  \
  template <typename CaseType>                                    \
  void FIXTURE##_##NAME##_DD::DoSingleCase(const CaseType& CASES##_case)

// ===================================================================

class KafkaBinaryReaderTest : public testing::Test {
 protected:
  // Buffer used during most of the tests. This assumes tests run sequentially.
  static constexpr int kBufferSize = 1024 * 64;
  static uint8_t buffer_[kBufferSize];
};

uint8_t KafkaBinaryReaderTest::buffer_[KafkaBinaryReaderTest::kBufferSize];

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

TEST_1D(KafkaBinaryReaderTest, ReadVarint32, kVarintCases) {
  ::memcpy(buffer_, kVarintCases_case.bytes, kVarintCases_case.size);

  {
    hcoona::minikafka::KafkaBinaryReader reader(
        absl::MakeConstSpan(buffer_, sizeof(buffer_)));

    uint32_t value;
    ONE_ASSERT_STATUS_OK(reader.ReadVarint32(&value));
    EXPECT_EQ(static_cast<uint32_t>(kVarintCases_case.value), value);
  }
}

TEST_1D(KafkaBinaryReaderTest, ReadVarint64, kVarintCases) {
  ::memcpy(buffer_, kVarintCases_case.bytes, kVarintCases_case.size);

  {
    hcoona::minikafka::KafkaBinaryReader reader(
        absl::MakeConstSpan(buffer_, sizeof(buffer_)));

    uint64_t value;
    ONE_ASSERT_STATUS_OK(reader.ReadVarint64(&value));
    EXPECT_EQ(kVarintCases_case.value, value);
  }
}

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

TEST_1D(KafkaBinaryReaderTest, ReadVarint32Error, kVarintErrorCases) {
  ::memcpy(buffer_, kVarintErrorCases_case.bytes, kVarintErrorCases_case.size);

  hcoona::minikafka::KafkaBinaryReader reader(
      absl::MakeConstSpan(buffer_, kVarintErrorCases_case.size));

  uint32_t value;
  EXPECT_EQ(kVarintErrorCases_case.can_parse, reader.ReadVarint32(&value).ok());
}

TEST_1D(KafkaBinaryReaderTest, ReadVarint32Error_LeavesValueInInitializedState,
        kVarintErrorCases) {
  ::memcpy(buffer_, kVarintErrorCases_case.bytes, kVarintErrorCases_case.size);

  hcoona::minikafka::KafkaBinaryReader reader(
      absl::MakeConstSpan(buffer_, kVarintErrorCases_case.size));

  uint32_t value = 0;
  EXPECT_EQ(kVarintErrorCases_case.can_parse, reader.ReadVarint32(&value).ok());
  // While the specific value following a failure is not critical, we do want to
  // ensure that it doesn't get set to an uninitialized value. (This check fails
  // in MSAN mode if value has been set to an uninitialized value.)
  EXPECT_EQ(value, value);
}

TEST_1D(KafkaBinaryReaderTest, ReadVarint64Error, kVarintErrorCases) {
  ::memcpy(buffer_, kVarintErrorCases_case.bytes, kVarintErrorCases_case.size);

  hcoona::minikafka::KafkaBinaryReader reader(
      absl::MakeConstSpan(buffer_, kVarintErrorCases_case.size));

  uint64_t value;
  EXPECT_EQ(kVarintErrorCases_case.can_parse, reader.ReadVarint64(&value).ok());
}

TEST_1D(KafkaBinaryReaderTest, ReadVarint64Error_LeavesValueInInitializedState,
        kVarintErrorCases) {
  ::memcpy(buffer_, kVarintErrorCases_case.bytes, kVarintErrorCases_case.size);

  hcoona::minikafka::KafkaBinaryReader reader(
      absl::MakeConstSpan(buffer_, kVarintErrorCases_case.size));

  uint64_t value = 0;
  EXPECT_EQ(kVarintErrorCases_case.can_parse, reader.ReadVarint64(&value).ok());
  // While the specific value following a failure is not critical, we do want to
  // ensure that it doesn't get set to an uninitialized value. (This check fails
  // in MSAN mode if value has been set to an uninitialized value.)
  EXPECT_EQ(value, value);
}
