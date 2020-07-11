// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/crypto/secure_hash.h"

#include <stddef.h>

#include "glog/logging.h"
#include "openssl/mem.h"
#include "openssl/sha.h"
#include "gtl/crypto/openssl_util.h"

namespace crypto {

namespace {

class SecureHashSHA256 : public SecureHash {
 public:
  SecureHashSHA256() {
    // Ensure that CPU features detection is performed before using
    // BoringSSL. This will enable hw accelerated implementations.
    EnsureOpenSSLInit();
    SHA256_Init(&ctx_);
  }

  SecureHashSHA256(const SecureHashSHA256& other) {
    memcpy(&ctx_, &other.ctx_, sizeof(ctx_));
  }

  ~SecureHashSHA256() override {
    OPENSSL_cleanse(&ctx_, sizeof(ctx_));
  }

  void Update(const void* input, size_t len) override {
    SHA256_Update(&ctx_, static_cast<const unsigned char*>(input), len);
  }

  void Finish(void* output, size_t len) override {
    ScopedOpenSSLSafeSizeBuffer<SHA256_DIGEST_LENGTH> result(
        static_cast<unsigned char*>(output), len);
    SHA256_Final(result.safe_buffer(), &ctx_);
  }

  std::unique_ptr<SecureHash> Clone() const override {
    return std::make_unique<SecureHashSHA256>(*this);
  }

  size_t GetHashLength() const override { return SHA256_DIGEST_LENGTH; }

 private:
  SHA256_CTX ctx_;
};

}  // namespace

std::unique_ptr<SecureHash> SecureHash::Create(Algorithm algorithm) {
  switch (algorithm) {
    case SHA256:
      return std::make_unique<SecureHashSHA256>();
    default:
      DCHECK(false) << "Not implemented";
      return nullptr;
  }
}

}  // namespace crypto
