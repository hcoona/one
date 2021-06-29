#include "one/codelab/protobuf_encoder/encoder.h"

#include <algorithm>

namespace codelab {

ssize_t EncodeTag(uint32_t field_number, WireType type,
                  gsl::span<gsl::byte> buffer) {
  return EncodeVarint((field_number << 3) | static_cast<uint32_t>(type),
                      buffer);
}

ssize_t EncodeVarint(uint64_t value, gsl::span<gsl::byte> buffer) {
  if (value < 128) {
    buffer[0] = static_cast<gsl::byte>(value);
    return 1;
  }

  size_t i = 0;
  while (value) {
    gsl::byte byte = static_cast<gsl::byte>(value & 0x7fU);
    value >>= 7;
    if (value) byte |= static_cast<gsl::byte>(0x80U);
    buffer[i++] = byte;
  }
  return i;
}

ssize_t ZigZagEncode32(int32_t value, gsl::span<gsl::byte> buffer) {
  uint32_t encoded_value = ((uint32_t)value << 1) ^ (value >> 31);
  return EncodeVarint(encoded_value, buffer);
}

ssize_t ZigZagEncode64(int64_t value, gsl::span<gsl::byte> buffer) {
  uint64_t encoded_value = ((uint64_t)value << 1) ^ (value >> 63);
  return EncodeVarint(encoded_value, buffer);
}

ssize_t EncodeString(gsl::cstring_span<> value, gsl::span<gsl::byte> buffer) {
  auto value_bytes = gsl::as_bytes(value);
  ssize_t count = EncodeVarint(value_bytes.size(), buffer);
  buffer = buffer.subspan(count);
  std::copy(std::begin(value_bytes), std::end(value_bytes), std::begin(buffer));
  return count + value_bytes.size();
}

}  // namespace codelab
