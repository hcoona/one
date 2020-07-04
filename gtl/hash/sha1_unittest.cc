// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/hash/sha1.h"

#include <stddef.h>

#include <string>

#include "gtest/gtest.h"

namespace {

template <class ElementType>
absl::Span<const uint8_t> as_uint8_bytes(absl::Span<ElementType> s) noexcept {
  return {reinterpret_cast<const uint8_t*>(s.data()),
          s.size() * sizeof(ElementType)};
}

}  // namespace

TEST(SHA1Test, Test1) {
  // Example A.1 from FIPS 180-2: one-block message.
  std::string input = "abc";

  static constexpr int kExpected[] = {0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81,
                                      0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78, 0x50,
                                      0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d};

  std::string output = gtl::SHA1HashString(input);
  ASSERT_EQ(gtl::kSHA1Length, output.size());
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output[i] & 0xFF);
}

TEST(SHA1Test, Test2) {
  // Example A.2 from FIPS 180-2: multi-block message.
  std::string input =
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

  static constexpr int kExpected[] = {0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2,
                                      0x6e, 0xba, 0xae, 0x4a, 0xa1, 0xf9, 0x51,
                                      0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1};

  std::string output = gtl::SHA1HashString(input);
  ASSERT_EQ(gtl::kSHA1Length, output.size());
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output[i] & 0xFF);
}

TEST(SHA1Test, Test3) {
  // Example A.3 from FIPS 180-2: long message.
  std::string input(1000000, 'a');

  static constexpr int kExpected[] = {0x34, 0xaa, 0x97, 0x3c, 0xd4, 0xc4, 0xda,
                                      0xa4, 0xf6, 0x1e, 0xeb, 0x2b, 0xdb, 0xad,
                                      0x27, 0x31, 0x65, 0x34, 0x01, 0x6f};

  std::string output = gtl::SHA1HashString(input);
  ASSERT_EQ(gtl::kSHA1Length, output.size());
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output[i] & 0xFF);
}

TEST(SHA1Test, Test1BytesAndSpan) {
  // Example A.1 from FIPS 180-2: one-block message.
  std::string input = "abc";
  unsigned char output[gtl::kSHA1Length];

  static constexpr unsigned char kExpected[] = {
      0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e,
      0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d};

  gtl::SHA1HashBytes(reinterpret_cast<const unsigned char*>(input.c_str()),
                      input.size(), output);
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output[i]);

  gtl::SHA1Digest output_array =
      gtl::SHA1HashSpan(as_uint8_bytes(absl::MakeSpan(input)));
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output_array[i]);
}

TEST(SHA1Test, Test2BytesAndSpan) {
  // Example A.2 from FIPS 180-2: multi-block message.
  std::string input =
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  unsigned char output[gtl::kSHA1Length];

  static constexpr unsigned char kExpected[] = {
      0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e, 0xba, 0xae,
      0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1};

  gtl::SHA1HashBytes(reinterpret_cast<const unsigned char*>(input.c_str()),
                      input.size(), output);
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output[i]);

  gtl::SHA1Digest output_array =
      gtl::SHA1HashSpan(as_uint8_bytes(absl::MakeSpan(input)));
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output_array[i]);
}

TEST(SHA1Test, Test3BytesAndSpan) {
  // Example A.3 from FIPS 180-2: long message.
  std::string input(1000000, 'a');
  unsigned char output[gtl::kSHA1Length];

  static constexpr unsigned char kExpected[] = {
      0x34, 0xaa, 0x97, 0x3c, 0xd4, 0xc4, 0xda, 0xa4, 0xf6, 0x1e,
      0xeb, 0x2b, 0xdb, 0xad, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6f};

  gtl::SHA1HashBytes(reinterpret_cast<const unsigned char*>(input.c_str()),
                      input.size(), output);
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output[i]);

  gtl::SHA1Digest output_array =
      gtl::SHA1HashSpan(as_uint8_bytes(absl::MakeSpan(input)));
  for (size_t i = 0; i < gtl::kSHA1Length; i++)
    EXPECT_EQ(kExpected[i], output_array[i]);
}
