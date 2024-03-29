// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.
//
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#pragma once

#include <atomic>
#include <functional>

#include "absl/time/time.h"
#include "one/jinduo/net/internal/channel.h"
#include "one/jinduo/net/internal/socket.h"

namespace jinduo {
namespace net {

class EventLoop;
class InetAddress;

// Acceptor of incoming TCP connections.
class Acceptor {
 public:
  using NewConnectionCallback =
      std::function<void(int /*fd*/, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listen_address, bool reuse_port);
  ~Acceptor();

  // Disallow copy.
  Acceptor(const Acceptor&) noexcept = delete;
  Acceptor& operator=(const Acceptor&) noexcept = delete;

  // Disallow move.
  Acceptor(Acceptor&&) noexcept = delete;
  Acceptor& operator=(Acceptor&&) noexcept = delete;

  void SetNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_callback_ = cb;
  }

  void Listen();

  [[nodiscard]] bool listening() const {
    return listening_.load(std::memory_order_acquire);
  }

 private:
  void HandleRead(absl::Time /*receive_time*/);

  EventLoop* loop_;
  Socket accept_socket_;
  Channel accept_channel_;
  int idle_fd_;

  NewConnectionCallback new_connection_callback_;

  std::atomic<bool> listening_{false};
};

}  // namespace net
}  // namespace jinduo
