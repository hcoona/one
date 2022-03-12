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

#include <vector>

#include "absl/time/time.h"
#include "one/jinduo/net/internal/poller.h"

struct pollfd;

namespace jinduo {
namespace net {

// IO Multiplexing with poll(2).
class PollPoller : public Poller {
 public:
  explicit PollPoller(EventLoop* loop);
  ~PollPoller() override;

  // Disallow copy.
  PollPoller(const PollPoller&) noexcept = delete;
  PollPoller& operator=(const PollPoller&) noexcept = delete;

  // Allow move but not implemented yet.
  PollPoller(PollPoller&&) noexcept = delete;
  PollPoller& operator=(PollPoller&&) noexcept = delete;

  absl::Time poll(int timeoutMs, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;

  using PollFdList = std::vector<struct pollfd>;
  PollFdList pollfds_;
};

}  // namespace net
}  // namespace jinduo
