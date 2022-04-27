/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "quic/server/QuicUDPSocketFactory.h"

namespace quic {

class QuicSharedUDPSocketFactory : public QuicUDPSocketFactory {
 public:
  ~QuicSharedUDPSocketFactory() override {}
  QuicSharedUDPSocketFactory() {}

  std::unique_ptr<folly::AsyncUDPSocket> make(folly::EventBase* evb, int fd)
      override {
    auto sock = std::make_unique<folly::AsyncUDPSocket>(evb);
    if (fd != -1) {
      sock->setFD(
          folly::NetworkSocket::fromFd(fd),
          folly::AsyncUDPSocket::FDOwnership::SHARED);
      sock->setDFAndTurnOffPMTU();
    }
    return sock;
  }
};
} // namespace quic
