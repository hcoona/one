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

struct epoll_event;

namespace jinduo {
namespace net {

// IO Multiplexing with epoll(4).
class EPollPoller : public Poller {
 public:
  explicit EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  // Disallow copy.
  EPollPoller(const EPollPoller&) noexcept = delete;
  EPollPoller& operator=(const EPollPoller&) noexcept = delete;

  // Allow move.
  EPollPoller(EPollPoller&&) noexcept = default;
  EPollPoller& operator=(EPollPoller&&) noexcept = default;

  absl::Time Poll(int timeout_ms,
                  std::vector<Channel*>* active_channels) override;

  void UpdateChannel(Channel* channel) override;
  void RemoveChannel(Channel* channel) override;

 private:
  void FillActiveChannels(int events_num,
                          std::vector<Channel*>* active_channels) const;
  void Update(int operation, Channel* channel) const;

  int epoll_fd_{-1};
  std::vector<epoll_event> events_;
};

}  // namespace net
}  // namespace jinduo
