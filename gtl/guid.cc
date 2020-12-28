// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/guid.h"

#include <stddef.h>
#include <stdint.h>

#include "absl/random/random.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"

namespace gtl {

namespace {

template <typename Char>
bool IsLowerHexDigit(Char c) {
  return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

template <typename StringPieceType>
bool IsValidGUIDInternal(StringPieceType guid, bool strict) {
  using CharType = typename StringPieceType::value_type;

  const size_t kGUIDLength = 36U;
  if (guid.length() != kGUIDLength) return false;

  for (size_t i = 0; i < guid.length(); ++i) {
    CharType current = guid[i];
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (current != '-') return false;
    } else {
      if (strict ? !IsLowerHexDigit(current) : !absl::ascii_isxdigit(current))
        return false;
    }
  }

  return true;
}

}  // namespace

std::string GenerateGUID() {
  uint64_t sixteen_bytes[2];
  absl::BitGen bitgen;
  sixteen_bytes[0] = bitgen.operator()();
  sixteen_bytes[1] = bitgen.operator()();

  // Set the GUID to version 4 as described in RFC 4122, section 4.4.
  // The format of GUID version 4 must be xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx,
  // where y is one of [8, 9, A, B].

  // Clear the version bits and set the version to 4:
  sixteen_bytes[0] &= 0xffffffff'ffff0fffULL;
  sixteen_bytes[0] |= 0x00000000'00004000ULL;

  // Set the two most significant bits (bits 6 and 7) of the
  // clock_seq_hi_and_reserved to zero and one, respectively:
  sixteen_bytes[1] &= 0x3fffffff'ffffffffULL;
  sixteen_bytes[1] |= 0x80000000'00000000ULL;

  return RandomDataToGUIDString(sixteen_bytes);
}

bool IsValidGUID(absl::string_view guid) {
  return IsValidGUIDInternal(guid, false /* strict */);
}

#ifdef ABSL_USES_STD_STRING_VIEW
bool IsValidGUID(std::u16string_view guid) {
  return IsValidGUIDInternal(guid, false /* strict */);
}
#endif  // ABSL_USES_STD_STRING_VIEW

bool IsValidGUIDOutputString(absl::string_view guid) {
  return IsValidGUIDInternal(guid, true /* strict */);
}

std::string RandomDataToGUIDString(const uint64_t bytes[2]) {
  return absl::StrFormat(
      "%08x-%04x-%04x-%04x-%012llx", static_cast<unsigned int>(bytes[0] >> 32),
      static_cast<unsigned int>((bytes[0] >> 16) & 0x0000ffff),
      static_cast<unsigned int>(bytes[0] & 0x0000ffff),
      static_cast<unsigned int>(bytes[1] >> 48),
      bytes[1] & 0x0000ffff'ffffffffULL);
}

}  // namespace gtl
