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

#include <set>
#include <utility>
#include <vector>

#include "one/jinduo/net/callbacks.h"
#include "one/jinduo/net/internal/channel.h"

namespace jinduo {
namespace net {

class EventLoop;
class Timer;
class TimerId;

// A best efforts timer queue.
// No guarantee that the callback will be on time.
class TimerQueue {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  // Disallow copy.
  TimerQueue(const TimerQueue&) noexcept = delete;
  TimerQueue& operator=(const TimerQueue&) noexcept = delete;

  // Allow move but not implemented yet.
  TimerQueue(TimerQueue&&) noexcept = delete;
  TimerQueue& operator=(TimerQueue&&) noexcept = delete;

  // Schedules the callback to be run at given time,
  // repeats if @c interval > 0.0.
  //
  // Must be thread safe. Usually be called from other threads.
  TimerId addTimer(TimerCallback cb, absl::Time when, absl::Duration interval);

  void cancel(TimerId timerId);

 private:
  // FIXME: use unique_ptr<Timer> instead of raw pointers.
  // This requires heterogeneous comparison lookup (N3465) from C++14
  // so that we can find an T* in a set<unique_ptr<T>>.
  using Entry = std::pair<absl::Time, Timer*>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void addTimerInLoop(Timer* timer);
  void cancelInLoop(TimerId timerId);
  // called when timerfd alarms
  void handleRead();
  // move out all expired timers
  std::vector<Entry> getExpired(absl::Time now);
  void reset(const std::vector<Entry>& expired, absl::Time now);

  bool insert(Timer* timer);

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  // Timer list sorted by expiration
  TimerList timers_;

  // for cancel()
  ActiveTimerSet activeTimers_;
  bool callingExpiredTimers_; /* atomic */
  ActiveTimerSet cancelingTimers_;
};

}  // namespace net
}  // namespace jinduo
