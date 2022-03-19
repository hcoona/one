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

#include <sstream>

#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"

namespace jinduo {
namespace net {

const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent =
    POLLIN | POLLPRI;  // NOLINT(hicpp-signed-bitwise)
const uint32_t Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false) {}

Channel::~Channel() {
  assert(!eventHandling_);
  assert(!addedToLoop_);
  if (loop_->IsInLoopThread()) {
    assert(!loop_->hasChannel(this));
  }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
  tie_ = obj;
  tied_ = true;
}

void Channel::update() {
  addedToLoop_ = true;
  loop_->updateChannel(this);
}

void Channel::remove() {
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::handleEvent(absl::Time receiveTime) {
  std::shared_ptr<void> guard;
  if (tied_) {
    guard = tie_.lock();
    if (guard) {
      handleEventWithGuard(receiveTime);
    }
  } else {
    handleEventWithGuard(receiveTime);
  }
}

void Channel::handleEventWithGuard(absl::Time receiveTime) {
  eventHandling_ = true;
  VLOG(1) << reventsToString();
  if (((revents_ & POLLHUP) != 0) &&  // NOLINT(hicpp-signed-bitwise)
      ((revents_ & POLLIN) == 0)) {   // NOLINT(hicpp-signed-bitwise)
    if (logHup_) {
      LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    }
    if (closeCallback_) {
      closeCallback_();
    }
  }

  if ((revents_ & POLLNVAL) != 0) {  // NOLINT(hicpp-signed-bitwise)
    LOG(WARNING) << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if ((revents_ & (POLLERR | POLLNVAL)) != 0) {  // NOLINT(hicpp-signed-bitwise)
    if (errorCallback_) {
      errorCallback_();
    }
  }
  if ((revents_ &                               // NOLINT(hicpp-signed-bitwise)
       (POLLIN | POLLPRI | POLLRDHUP)) != 0) {  // NOLINT(hicpp-signed-bitwise)
    if (readCallback_) {
      readCallback_(receiveTime);
    }
  }
  if ((revents_ & POLLOUT) != 0) {  // NOLINT(hicpp-signed-bitwise)
    if (writeCallback_) {
      writeCallback_();
    }
  }
  eventHandling_ = false;
}

std::string Channel::reventsToString() const {
  return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const {
  return eventsToString(fd_, events_);
}

std::string Channel::eventsToString(int fd, uint32_t ev) {
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

}  // namespace net
}  // namespace jinduo
