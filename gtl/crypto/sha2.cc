// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/crypto/sha2.h"

#include <stddef.h>

#include <memory>

#include "gtl/crypto/secure_hash.h"
#include "openssl/sha.h"

namespace gtl {
namespace crypto {

std::array<uint8_t, kSHA256Length> SHA256Hash(absl::Span<const uint8_t> input) {
  std::array<uint8_t, kSHA256Length> digest;
  ::SHA256(input.data(), input.size(), digest.data());
  return digest;
}

void SHA256HashString(absl::string_view str, void* output, size_t len) {
  std::unique_ptr<SecureHash> ctx(SecureHash::Create(SecureHash::SHA256));
  ctx->Update(str.data(), str.length());
  ctx->Finish(output, len);
}

std::string SHA256HashString(absl::string_view str) {
  std::string output(kSHA256Length, 0);
  SHA256HashString(str, &output[0], output.size());
  return output;
}

}  // namespace crypto
}  // namespace gtl
