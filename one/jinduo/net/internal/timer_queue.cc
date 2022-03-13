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

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "one/jinduo/net/internal/timer_queue.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include <algorithm>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/internal/timer.h"
#include "one/jinduo/net/timer_id.h"

namespace jinduo {
namespace net {
namespace details {

int createTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timerfd < 0) {
    LOG(FATAL) << "Failed in timerfd_create";
  }
  return timerfd;
}

struct timespec howMuchTimeFromNow(absl::Time when) {
  absl::Duration duration = when - absl::Now();
  static constexpr absl::Duration kDurationMin = absl::Microseconds(100);
  if (duration < kDurationMin) {
    duration = kDurationMin;
  }
  return absl::ToTimespec(duration);
}

void readTimerfd(int timerfd, absl::Time now) {
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  VLOG(1) << "TimerQueue::handleRead() " << howmany << " at " << now;
  if (n != sizeof howmany) {
    LOG(ERROR) << "TimerQueue::handleRead() reads " << n
               << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, absl::Time expiration) {
  // wake up loop by timerfd_settime()
  itimerspec newValue{};
  itimerspec oldValue{};
  ::memset(&newValue, 0, sizeof(newValue));
  ::memset(&oldValue, 0, sizeof(oldValue));
  newValue.it_value = howMuchTimeFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if (ret != 0) {
    LOG(ERROR) << "timerfd_settime()";
  }
}

}  // namespace details

TimerQueue::TimerQueue(EventLoop* loop)
    : loop_(loop),
      timerfd_(details::createTimerfd()),
      timerfdChannel_(loop, timerfd_),
      callingExpiredTimers_(false) {
  timerfdChannel_.setReadCallback(std::bind(  // NOLINT(modernize-avoid-bind):
                                              // By-design ignore timestamp arg
      &TimerQueue::handleRead, this));
  // we are always reading the timerfd, we disarm it with timerfd_settime.
  timerfdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  timerfdChannel_.disableAll();
  timerfdChannel_.remove();
  ::close(timerfd_);
  // do not remove channel, since we're in EventLoop::dtor();
  for (const Entry& timer : timers_) {
    delete timer.second;
  }
}

TimerId TimerQueue::addTimer(TimerCallback cb, absl::Time when,
                             absl::Duration interval) {
  auto* timer = new Timer(std::move(cb), when, interval);
  loop_->runInLoop(absl::bind_front(&TimerQueue::addTimerInLoop, this, timer));
  return {timer, timer->sequence()};
}

void TimerQueue::cancel(TimerId timerId) {
  loop_->runInLoop(absl::bind_front(&TimerQueue::cancelInLoop, this, timerId));
}

void TimerQueue::addTimerInLoop(Timer* timer) {
  loop_->assertInLoopThread();
  bool earliestChanged = insert(timer);

  if (earliestChanged) {
    details::resetTimerfd(timerfd_, timer->expiration());
  }
}

void TimerQueue::cancelInLoop(TimerId timerId) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  ActiveTimer timer(timerId.timer_, timerId.sequence_);
  auto it = activeTimers_.find(timer);
  if (it != activeTimers_.end()) {
    size_t n = timers_.erase(Entry(it->first->expiration(), it->first));
    assert(n == 1);
    (void)n;
    delete it->first;  // FIXME: no delete please
    activeTimers_.erase(it);
  } else if (callingExpiredTimers_) {
    cancelingTimers_.insert(timer);
  }
  assert(timers_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  absl::Time now = absl::Now();
  details::readTimerfd(timerfd_, now);

  std::vector<Entry> expired = getExpired(now);

  callingExpiredTimers_ = true;
  cancelingTimers_.clear();
  // safe to callback outside critical section
  for (const Entry& it : expired) {
    it.second->run();
  }
  callingExpiredTimers_ = false;

  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(absl::Time now) {
  assert(timers_.size() == activeTimers_.size());
  std::vector<Entry> expired;
  Entry sentry(now,
               reinterpret_cast<Timer*>(  // NOLINT(performance-no-int-to-ptr)
                   UINTPTR_MAX));
  auto end = timers_.lower_bound(sentry);
  assert(end == timers_.end() || now < end->first);
  std::copy(timers_.begin(), end, back_inserter(expired));
  timers_.erase(timers_.begin(), end);

  for (const Entry& it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    size_t n = activeTimers_.erase(timer);
    assert(n == 1);
    (void)n;
  }

  assert(timers_.size() == activeTimers_.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, absl::Time now) {
  absl::Time nextExpire;

  for (const Entry& it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    if (it.second->repeat() &&
        cancelingTimers_.find(timer) == cancelingTimers_.end()) {
      it.second->restart(now);
      insert(it.second);
    } else {
      // FIXME move to a free list
      delete it.second;  // FIXME: no delete please
    }
  }

  if (!timers_.empty()) {
    nextExpire = timers_.begin()->second->expiration();
  }

  if (nextExpire != absl::InfinitePast()) {
    details::resetTimerfd(timerfd_, nextExpire);
  }
}

bool TimerQueue::insert(Timer* timer) {
  loop_->assertInLoopThread();
  assert(timers_.size() == activeTimers_.size());
  bool earliestChanged = false;
  absl::Time when = timer->expiration();
  auto it = timers_.begin();
  if (it == timers_.end() || when < it->first) {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result =
        timers_.insert(Entry(when, timer));
    assert(result.second);
    (void)result;
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result =
        activeTimers_.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
    (void)result;
  }

  assert(timers_.size() == activeTimers_.size());
  return earliestChanged;
}

}  // namespace net
}  // namespace jinduo
