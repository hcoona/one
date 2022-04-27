/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "quic/codec/QuicConnectionId.h"

namespace quic {

/**
 * When ConnectionIdAlgo encodes a ConnectionID, Rejector gives the user a way
 * to reject the resulted ConnectionID. For example, a Quic server may have many
 * ongoing connections that all needs different ConnectionID. If the
 * ConnectionID generated by ConnectionIdAlgo is duplicated to an existing one,
 * the server can use this interface to reject the ConnectionID.
 */
class ServerConnectionIdRejector {
 public:
  virtual ~ServerConnectionIdRejector() = default;
  virtual bool rejectConnectionId(
      const ConnectionId& candidate) const noexcept = 0;
};

} // namespace quic
