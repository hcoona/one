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

#include <map>
#include <vector>

#include "absl/time/time.h"
#include "one/jinduo/net/event_loop.h"

namespace jinduo {
namespace net {

class Channel;

// Base class for IO multiplexing
//
// This class doesn't own the `Channel` objects.
class Poller {
 public:
  explicit Poller(EventLoop* loop);
  virtual ~Poller();

  // Disallow copy.
  Poller(const Poller&) noexcept = delete;
  Poller& operator=(const Poller&) noexcept = delete;

  // Allow move.
  Poller(Poller&&) noexcept = default;
  Poller& operator=(Poller&&) noexcept = default;

  static Poller* CreateDefaultPoller(EventLoop* loop);

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  virtual absl::Time Poll(int timeout_ms,
                          std::vector<Channel*>* active_channels) = 0;

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  virtual void UpdateChannel(Channel* channel) = 0;

  /// Remove the channel, when it destructs.
  /// Must be called in the loop thread.
  virtual void RemoveChannel(Channel* channel) = 0;

  virtual bool HasChannel(Channel* channel) const;

  void AssertInLoopThread() const { owner_loop_->AssertInLoopThread(); }

 protected:
  std::map<int /*fd*/, Channel*> channels_;

 private:
  EventLoop* owner_loop_;
};

}  // namespace net
}  // namespace jinduo
