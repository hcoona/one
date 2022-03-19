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
#include <memory>
#include <string>
#include <utility>

#include "absl/time/time.h"

namespace jinduo {
namespace net {

class EventLoop;

// A selectable I/O channel.
//
// This class doesn't own the file descriptor. The file descriptor could be a
// socket, an eventfd, a timerfd, or a signalfd
class Channel {
  static const uint32_t kNoneEvent;
  static const uint32_t kReadEvent;
  static const uint32_t kWriteEvent;

 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(absl::Time)>;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  // Disallow copy.
  Channel(const Channel&) noexcept = delete;
  Channel& operator=(const Channel&) noexcept = delete;

  // Disallow move.
  Channel(Channel&&) noexcept = delete;
  Channel& operator=(Channel&&) noexcept = delete;

  // Tie this channel to the owner object managed by shared_ptr,
  // prevent the owner object being destroyed in handleEvent.
  void Tie(const std::shared_ptr<void>& obj);

  //
  // Event handling.
  //

  void HandleEvent(absl::Time receive_time);

  void SetReadCallback(ReadEventCallback cb) { read_callback_ = std::move(cb); }
  void SetWriteCallback(EventCallback cb) { write_callback_ = std::move(cb); }
  void SetCloseCallback(EventCallback cb) { close_callback_ = std::move(cb); }
  void SetErrorCallback(EventCallback cb) { error_callback_ = std::move(cb); }

  //
  // Status accessors & modifiers.
  //

  [[nodiscard]] int fd() const { return fd_; }

  // Requested events for poller. See poll(2) manpage.
  [[nodiscard]] uint32_t events() const { return events_; }

  // Returned events from poller. See poll(2) manpage.
  [[nodiscard]] uint32_t revents() const { return revents_; }
  // Set returned events by poller.
  void set_revents(uint32_t revents) { revents_ = revents; }

  [[nodiscard]] bool IsNoneEvent() const { return events_ == kNoneEvent; }

  void EnableReading() {
    events_ |= kReadEvent;
    Update();
  }
  void DisableReading() {
    events_ &= ~kReadEvent;
    Update();
  }

  void EnableWriting() {
    events_ |= kWriteEvent;
    Update();
  }
  void DisableWriting() {
    events_ &= ~kWriteEvent;
    Update();
  }

  void DisableAll() {
    events_ = kNoneEvent;
    Update();
  }

  [[nodiscard]] bool IsReadingEnabled() const {
    return (events_ & kReadEvent) != 0U;
  }
  [[nodiscard]] bool IsWritingEnabled() const {
    return (events_ & kWriteEvent) != 0U;
  }

  //
  // For poller
  //

  [[nodiscard]] int index() const { return index_; }
  void set_index(int idx) { index_ = idx; }

  //
  // For debugging
  //

  [[nodiscard]] std::string REventsToString() const;
  [[nodiscard]] std::string EventsToString() const;

  void DisableLogHup() {
    log_hup_enabled_.store(false, std::memory_order_release);
  }

  //
  // For EventLoop
  //

  EventLoop* owner_event_loop() { return loop_; }
  void RemoveFromOwnerEventLoop();

 private:
  void Update();

  void HandleEventWithGuard(absl::Time receiveTime);

  EventLoop* loop_;
  int fd_;
  uint32_t events_{0};
  uint32_t revents_{0};  // it's the received event types of epoll or poll
  int index_{-1};        // used by Poller.
  std::atomic<bool> log_hup_enabled_{true};

  std::weak_ptr<void> tie_;
  bool tied_{false};

  std::atomic<bool> event_handling_{false};
  std::atomic<bool> added_to_loop_{false};

  ReadEventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;
};

}  // namespace net
}  // namespace jinduo
