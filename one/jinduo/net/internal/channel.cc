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

#include "one/jinduo/net/internal/channel.h"

#include <poll.h>

#include <atomic>
#include <memory>
#include <sstream>

#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"

namespace jinduo {
namespace net {

const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent =
    POLLIN | POLLPRI;  // NOLINT(hicpp-signed-bitwise)
const uint32_t Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) : loop_(loop), fd_(fd) {}

Channel::~Channel() {
  assert(!event_handling_.load(std::memory_order_acquire));
  assert(!added_to_loop_.load(std::memory_order_acquire));

  if (loop_->IsInLoopThread()) {
    assert(!loop_->HasChannel(this));
  }
}

void Channel::Tie(const std::shared_ptr<void>& obj) {
  tie_ = std::weak_ptr<void>(obj);
  tied_ = true;
}

void Channel::HandleEvent(absl::Time receive_time) {
  std::shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      HandleEventWithGuard(receive_time);
    }
  } else {
    HandleEventWithGuard(receive_time);
  }
}

void Channel::HandleEventWithGuard(absl::Time receiveTime) {
  event_handling_.store(true, std::memory_order_release);

  VLOG(1) << REventsToString();

  if (((revents_ & POLLHUP) != 0) &&  // NOLINT(hicpp-signed-bitwise)
      ((revents_ & POLLIN) == 0)) {   // NOLINT(hicpp-signed-bitwise)
    if (log_hup_enabled_) {
      LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (close_callback_) {
      close_callback_();
    }
  }

  if ((revents_ & POLLNVAL) != 0) {  // NOLINT(hicpp-signed-bitwise)
    LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if ((revents_ & (POLLERR | POLLNVAL)) != 0) {  // NOLINT(hicpp-signed-bitwise)
    if (error_callback_) {
      error_callback_();
    }
  }
  if ((revents_ &                               // NOLINT(hicpp-signed-bitwise)
       (POLLIN | POLLPRI | POLLRDHUP)) != 0) {  // NOLINT(hicpp-signed-bitwise)
    if (read_callback_) {
      read_callback_(receiveTime);
    }
  }
  if ((revents_ & POLLOUT) != 0) {  // NOLINT(hicpp-signed-bitwise)
    if (write_callback_) {
      write_callback_();
    }
  }

  event_handling_.store(false, std::memory_order_release);
}

void Channel::RemoveFromOwnerEventLoop() {
  assert(IsNoneEvent());

  added_to_loop_.store(false, std::memory_order_release);
  loop_->RemoveChannel(this);
}

void Channel::Update() {
  added_to_loop_.store(true, std::memory_order_release);
  loop_->UpdateChannel(this);
}

namespace {

std::string EventsToStringInternal(int fd, uint32_t ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if ((ev & POLLIN) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "IN ";
  }
  if ((ev & POLLPRI) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "PRI ";
  }
  if ((ev & POLLOUT) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "OUT ";
  }
  if ((ev & POLLHUP) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "HUP ";
  }
  if ((ev & POLLRDHUP) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "RDHUP ";
  }
  if ((ev & POLLERR) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "ERR ";
  }
  if ((ev & POLLNVAL) != 0) {  // NOLINT(hicpp-signed-bitwise)
    oss << "NVAL ";
  }

  return oss.str();
}

}  // namespace

std::string Channel::REventsToString() const {
  return EventsToStringInternal(fd_, revents_);
}

std::string Channel::EventsToString() const {
  return EventsToStringInternal(fd_, events_);
}

}  // namespace net
}  // namespace jinduo
