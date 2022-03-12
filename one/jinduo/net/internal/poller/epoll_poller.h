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

  // Allow move but not implemented yet.
  EPollPoller(EPollPoller&&) noexcept = delete;
  EPollPoller& operator=(EPollPoller&&) noexcept = delete;

  absl::Time poll(int timeoutMs, ChannelList* activeChannels) override;
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
  static const int kInitEventListSize = 16;

  static const char* operationToString(int op);

  void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
  void update(int operation, Channel* channel);

  using EventList = std::vector<struct epoll_event>;

  int epollfd_;
  EventList events_;
};

}  // namespace net
}  // namespace jinduo
