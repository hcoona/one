#include "one/codelab/protobuf_encoder/encoder.h"

#include <array>

#include "gtest/gtest.h"

namespace codelab {
namespace {

TEST(PbEncoderTest, TestEncodeVarint_1) {
  std::array<gsl::byte, 1> expected = {static_cast<gsl::byte>(0x01U)};

  std::array<gsl::byte, 10> actual;
  ssize_t size = EncodeVarint(1, actual);

  ASSERT_EQ(expected.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

TEST(PbEncoderTest, TestEncodeVarint_300) {
  std::array<gsl::byte, 2> expected = {static_cast<gsl::byte>(0b10101100),
                                       static_cast<gsl::byte>(0b00000010)};

  std::array<gsl::byte, 10> actual;
  ssize_t size = EncodeVarint(300, actual);

  ASSERT_EQ(expected.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

// Encode following protobuf description:
// message Test1 {
//   optional int32 a = 1;
// }
// In which the value of a is 150.
TEST(PbEncoderTest, TestEncodeMessageVarint_150) {
  std::array<gsl::byte, 3> expected = {static_cast<gsl::byte>(0x08U),
                                       static_cast<gsl::byte>(0x96U),
                                       static_cast<gsl::byte>(0x01U)};

  std::array<gsl::byte, 10> actual;
  gsl::span<gsl::byte> buffer = actual;
  ssize_t size1 = EncodeTag(1, WireType::kVarint, buffer);
  buffer = buffer.subspan(size1);
  ssize_t size2 = EncodeVarint(150, buffer);
  ssize_t size = size1 + size2;

  ASSERT_EQ(expected.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

TEST(PbEncoderTest, TestZigZagEncode32_m1) {
  std::array<gsl::byte, 1> expected = {static_cast<gsl::byte>(0x01)};

  std::array<gsl::byte, 10> actual;
  ssize_t size = ZigZagEncode32(-1, actual);

  ASSERT_EQ(expected.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

// Encode following protobuf description:
// message Test2 {
//   optional string b = 2;
// }
// In which the value of b is "testing".
TEST(PbEncoderTest, TestEncodeString) {
  std::array<gsl::byte, 9> expected = {
      static_cast<gsl::byte>(0x12), static_cast<gsl::byte>(0x07),
      static_cast<gsl::byte>(0x74), static_cast<gsl::byte>(0x65),
      static_cast<gsl::byte>(0x73), static_cast<gsl::byte>(0x74),
      static_cast<gsl::byte>(0x69), static_cast<gsl::byte>(0x6e),
      static_cast<gsl::byte>(0x67),
  };

  std::array<gsl::byte, 20> actual;
  gsl::span<gsl::byte> buffer = actual;
  ssize_t size1 = EncodeTag(2, WireType::kDelimited, buffer);
  buffer = buffer.subspan(size1);
  ssize_t size2 = EncodeString(gsl::ensure_z("testing"), buffer);
  ssize_t size = size1 + size2;

  ASSERT_EQ(expected.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

// Encode following protobuf description:
// message Test1 {
//   optional int32 a = 1;
// }
// message Test3 {
//   optional Test1 c = 3;
// }
// In which the value of a is 150.
TEST(PbEncoderTest, TestEncodeNestedMessage) {
  std::array<gsl::byte, 5> expected = {
      static_cast<gsl::byte>(0x1aU), static_cast<gsl::byte>(0x03U),
      static_cast<gsl::byte>(0x08U), static_cast<gsl::byte>(0x96U),
      static_cast<gsl::byte>(0x01U)};

  std::array<gsl::byte, 10> actual;
  ssize_t size = 0;

  gsl::span<gsl::byte> buffer = actual;
  size += EncodeTag(3, WireType::kDelimited, buffer);

  ssize_t message_size_hole_pos = size;
  size += 1;  // Leave a hole for size of the message Test3.

  buffer = gsl::span<gsl::byte>(actual).subspan(size);
  size += EncodeTag(1, WireType::kVarint, buffer);

  buffer = gsl::span<gsl::byte>(actual).subspan(size);
  size += EncodeVarint(150, buffer);

  buffer = gsl::span<gsl::byte>(actual).subspan(message_size_hole_pos);
  EncodeVarint(size - message_size_hole_pos - 1, buffer);  // Fill the hole.

  ASSERT_EQ(expected.size(), size);
  for (int i = 0; i < size; i++) {
    EXPECT_EQ(expected[i], actual[i]);
  }
}

}  // namespace
}  // namespace codelab
