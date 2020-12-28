#pragma once

#include <cstddef>

#include "gsl/gsl"

namespace codelab {

// 最终使用的时候，可以考虑从后往前填写，这样比较简单，例如：
// 1. Enlarge 字符串大小，然后拷贝字符串
// 2. 估算字符串大小 Encode 之后占几字节，Enlarge，然后写 Varint

enum class WireType : uint32_t {
  kVarint = 0,
  k64Bit = 1,
  kDelimited = 2,
  k32Bit = 5,
};

ssize_t EncodeTag(uint32_t field_number, WireType type,
                  gsl::span<gsl::byte> buffer);

ssize_t EncodeVarint(uint64_t value, gsl::span<gsl::byte> buffer);

ssize_t ZigZagEncode32(int32_t value, gsl::span<gsl::byte> buffer);

ssize_t ZigZagEncode64(int64_t value, gsl::span<gsl::byte> buffer);

ssize_t EncodeString(gsl::cstring_span<> value, gsl::span<gsl::byte> buffer);

}  // namespace codelab
