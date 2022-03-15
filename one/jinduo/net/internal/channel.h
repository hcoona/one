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

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "absl/time/time.h"

namespace jinduo {
namespace net {

class EventLoop;

// A selectable I/O channel.
//
// This class doesn't own the file descriptor.
// The file descriptor could be a socket,
// an eventfd, a timerfd, or a signalfd
class Channel {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(absl::Time)>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  // Disallow copy.
  Channel(const Channel&) noexcept = delete;
  Channel& operator=(const Channel&) noexcept = delete;

  // Allow move but not implemented yet.
  Channel(Channel&&) noexcept = delete;
  Channel& operator=(Channel&&) noexcept = delete;

  void handleEvent(absl::Time receiveTime);
  void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const std::shared_ptr<void>&);

  [[nodiscard]] int fd() const { return fd_; }
  [[nodiscard]] uint32_t events() const { return events_; }
  void set_revents(uint32_t revt) { revents_ = revt; }  // used by pollers
  // int revents() const { return revents_; }
  [[nodiscard]] bool isNoneEvent() const { return events_ == kNoneEvent; }

  void enableReading() {
    events_ |= kReadEvent;
    update();
  }
  void disableReading() {
    events_ &= ~kReadEvent;
    update();
  }
  void enableWriting() {
    events_ |= kWriteEvent;
    update();
  }
  void disableWriting() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disableAll() {
    events_ = kNoneEvent;
    update();
  }
  [[nodiscard]] bool isWriting() const { return (events_ & kWriteEvent) != 0U; }
  [[nodiscard]] bool isReading() const { return (events_ & kReadEvent) != 0U; }

  // for Poller
  [[nodiscard]] int index() const { return index_; }
  void set_index(int idx) { index_ = idx; }

  // for debug
  [[nodiscard]] std::string reventsToString() const;
  [[nodiscard]] std::string eventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  static std::string eventsToString(int fd, uint32_t ev);

  void update();
  void handleEventWithGuard(absl::Time receiveTime);

  static const uint32_t kReadEvent;
  static const uint32_t kNoneEvent;
  static const uint32_t kWriteEvent;

  EventLoop* loop_;
  const int fd_;
  uint32_t events_;
  uint32_t revents_;  // it's the received event types of epoll or poll
  int index_;         // used by Poller.
  bool logHup_;

  std::weak_ptr<void> tie_;
  bool tied_;
  bool eventHandling_;
  bool addedToLoop_;

  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}  // namespace net
}  // namespace jinduo
