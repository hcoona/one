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

#include "one/jinduo/net/internal/poller/poll_poller.h"

#include <poll.h>

#include <cassert>
#include <cerrno>

#include "absl/time/clock.h"
#include "glog/logging.h"
#include "one/jinduo/net/internal/channel.h"

namespace jinduo {
namespace net {

PollPoller::PollPoller(EventLoop* loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

absl::Time PollPoller::Poll(int timeout_ms,
                            std::vector<Channel*>* active_channels) {
  int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout_ms);
  int savedErrno = errno;
  absl::Time now(absl::Now());
  if (numEvents > 0) {
    VLOG(1) << numEvents << " events happened";
    FillActiveChannels(numEvents, active_channels);
  } else if (numEvents == 0) {
    VLOG(1) << " nothing happened";
  } else {
    if (savedErrno != EINTR) {
      errno = savedErrno;
      PLOG(ERROR) << "PollPoller::poll()";
    }
  }
  return now;
}

void PollPoller::UpdateChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  VLOG(1) << "fd = " << channel->fd() << " events = " << channel->events();
  if (channel->index() < 0) {
    // a new one, add to pollfds_
    assert(channels_.find(channel->fd()) == channels_.end());
    pollfd pfd{};
    pfd.fd = channel->fd();
    // NOLINTNEXTLINE(google-runtime-int)
    pfd.events = static_cast<short>(channel->events());  // NOLINT(runtime/int)
    pfd.revents = 0;
    pollfds_.push_back(pfd);
    int idx = static_cast<int>(pollfds_.size()) - 1;
    channel->set_index(idx);
    channels_[pfd.fd] = channel;
  } else {
    // update existing one
    assert(channels_.find(channel->fd()) != channels_.end());
    assert(channels_[channel->fd()] == channel);
    int idx = channel->index();
    assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
    struct pollfd& pfd = pollfds_[idx];
    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
    pfd.fd = channel->fd();
    // NOLINTNEXTLINE(google-runtime-int)
    pfd.events = static_cast<short>(channel->events());  // NOLINT(runtime/int)
    pfd.revents = 0;
    if (channel->IsNoneEvent()) {
      // ignore this pollfd
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void PollPoller::RemoveChannel(Channel* channel) {
  Poller::AssertInLoopThread();
  VLOG(1) << "fd = " << channel->fd();
  assert(channels_.find(channel->fd()) != channels_.end());
  assert(channels_[channel->fd()] == channel);
  assert(channel->IsNoneEvent());
  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
  const struct pollfd& pfd = pollfds_[idx];
  (void)pfd;
  assert(pfd.fd == -channel->fd() - 1 &&
         static_cast<uint32_t>(pfd.events) == channel->events());
  size_t n = channels_.erase(channel->fd());
  assert(n == 1);
  (void)n;
  if (absl::implicit_cast<size_t>(idx) == pollfds_.size() - 1) {
    pollfds_.pop_back();
  } else {
    int channelAtEnd = pollfds_.back().fd;
    iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
    if (channelAtEnd < 0) {
      channelAtEnd = -channelAtEnd - 1;
    }
    channels_[channelAtEnd]->set_index(idx);
    pollfds_.pop_back();
  }
}

void PollPoller::FillActiveChannels(
    int numEvents, std::vector<Channel*>* active_channels) const {
  for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && numEvents > 0;
       ++pfd) {
    if (pfd->revents > 0) {
      --numEvents;
      auto ch = channels_.find(pfd->fd);
      assert(ch != channels_.end());
      Channel* channel = ch->second;
      assert(channel->fd() == pfd->fd);
      channel->set_revents(pfd->revents);
      // pfd->revents = 0;
      active_channels->push_back(channel);
    }
  }
}

}  // namespace net
}  // namespace jinduo
