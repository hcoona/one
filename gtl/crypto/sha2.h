// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GTL_CRYPTO_SHA2_H_
#define GTL_CRYPTO_SHA2_H_

#include <stddef.h>

#include <array>
#include <string>

#include "third_party/absl/types/span.h"
#include "third_party/absl/strings/string_view.h"

namespace gtl {
namespace crypto {

// These functions perform SHA-256 operations.
//
// Functions for SHA-384 and SHA-512 can be added when the need arises.

static const size_t kSHA256Length = 32;  // Length in bytes of a SHA-256 hash.

// Computes the SHA-256 hash of |input|.
std::array<uint8_t, kSHA256Length> SHA256Hash(
    absl::Span<const uint8_t> input);

// Convenience version of the above that returns the result in a 32-byte
// string.
std::string SHA256HashString(absl::string_view str);

// Computes the SHA-256 hash of the input string 'str' and stores the first
// 'len' bytes of the hash in the output buffer 'output'.  If 'len' > 32,
// only 32 bytes (the full hash) are stored in the 'output' buffer.
void SHA256HashString(absl::string_view str,
                                    void* output,
                                    size_t len);

}  // namespace crypto
}  // namespace gtl

#endif  // GTL_CRYPTO_SHA2_H_
