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

#include "one/jinduo/net/internal/poller/epoll_poller.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "glog/logging.h"
#include "one/jinduo/net/internal/channel.h"

namespace jinduo {
namespace net {

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {

constexpr int kInitEventListSize = 16;

constexpr int kNew = -1;
constexpr int kAdded = 1;
constexpr int kDeleted = 2;

const char* OperationToString(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}

}  // namespace

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epoll_fd_ < 0) {
    PLOG(FATAL) << "Failed to epoll_create1().";
  }
}

EPollPoller::~EPollPoller() {
  if (epoll_fd_ >= 0) {
    ::close(epoll_fd_);
  }
}

absl::Time EPollPoller::Poll(int timeout_ms,
                             std::vector<Channel*>* active_channels) {
  VLOG(2) << "fd total count " << channels_.size();

  int events_num = ::epoll_wait(epoll_fd_, &*events_.begin(),
                                static_cast<int>(events_.size()), timeout_ms);
  int saved_errno = errno;

  absl::Time now(absl::Now());
  if (events_num > 0) {
    VLOG(2) << events_num << " events happened";

    FillActiveChannels(events_num, active_channels);

    if (absl::implicit_cast<size_t>(events_num) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  } else if (events_num == 0) {
    VLOG(3) << "nothing happened";
  } else {
    // error happens, log uncommon ones
    if (saved_errno != EINTR) {
      errno = saved_errno;
      PLOG(ERROR) << "Failed to epoll_wait().";
    }
  }

  return now;
}

void EPollPoller::FillActiveChannels(
    int events_num, std::vector<Channel*>* active_channels) const {
  assert(absl::implicit_cast<size_t>(events_num) <= events_.size());

  for (int i = 0; i < events_num; ++i) {
    auto* channel = reinterpret_cast<Channel*>(events_[i].data.ptr);

#ifndef NDEBUG
    int fd = channel->fd();
    auto it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif

    channel->set_revents(events_[i].events);
    active_channels->emplace_back(channel);
  }
}

void EPollPoller::UpdateChannel(Channel* channel) {
  Poller::AssertInLoopThread();

  const int index = channel->index();

  VLOG(1) << "fd=" << channel->fd() << ", events=" << channel->events()
          << ", index=" << index;

  if (index == kNew || index == kDeleted) {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();
    if (index == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {  // index == kDeleted
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    Update(EPOLL_CTL_ADD, channel);
  } else {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->IsNoneEvent()) {
      Update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    } else {
      Update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::RemoveChannel(Channel* channel) {
  Poller::AssertInLoopThread();

  int fd = channel->fd();
  VLOG(1) << "fd=" << fd;

  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->IsNoneEvent());

  int index = channel->index();
  assert(index == kAdded || index == kDeleted);

  size_t n = channels_.erase(fd);
  (void)n;
  assert(n == 1);

  if (index == kAdded) {
    Update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

void EPollPoller::Update(int operation, Channel* channel) const {
  epoll_event event{};
  ::memset(&event, 0, sizeof event);
  event.events = channel->events();
  event.data.ptr = reinterpret_cast<void*>(channel);

  int fd = channel->fd();
  VLOG(1) << "epoll_ctl op=" << OperationToString(operation) << ", fd=" << fd
          << ", event={" << channel->EventsToString() << "}";
  if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
    if (operation == EPOLL_CTL_DEL) {
      LOG(ERROR) << "epoll_ctl failed to DEL. fd=" << fd;
    } else {
      LOG(FATAL) << "epoll_ctl failed. op=" << OperationToString(operation)
                 << ", fd=" << fd;
    }
  }
}

}  // namespace net
}  // namespace jinduo
