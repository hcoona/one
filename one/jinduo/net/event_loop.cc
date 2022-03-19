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

#include "one/jinduo/net/event_loop.h"

#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>
#include <atomic>
#include <csignal>
#include <utility>

#include "absl/time/clock.h"
#include "glog/logging.h"
#include "glog/vlog_is_on.h"
#include "one/jinduo/net/internal/channel.h"
#include "one/jinduo/net/internal/poller.h"
#include "one/jinduo/net/internal/sockets_ops.h"
#include "one/jinduo/net/internal/timer_queue.h"

namespace jinduo {
namespace net {

namespace {

__thread EventLoop* this_thread_event_loop = nullptr;

constexpr int kPollTimeMs = 10000;

int CreateEventFd() {
  int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (fd < 0) {
    PLOG(FATAL) << "Failed to create eventfd.";
  }
  return fd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
 public:
  IgnoreSigPipe() {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    ::signal(SIGPIPE, SIG_IGN);
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe ignore_sigpipe;

}  // namespace

// static
EventLoop* EventLoop::GetCurrentThreadEventLoop() {
  return this_thread_event_loop;
}

EventLoop::EventLoop()
    : thread_id_(this_thread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timer_queue_(new TimerQueue(this)),
      wakeup_fd_(CreateEventFd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)) {
  VLOG(1) << "EventLoop created " << this << " in thread " << thread_id_;
  if (this_thread_event_loop != nullptr) {
    LOG(FATAL) << "Another EventLoop " << this_thread_event_loop
               << " exists in this thread " << thread_id_;
  } else {
    this_thread_event_loop = this;
  }
  wakeup_channel_->setReadCallback(
      [this](absl::Time /*ignored*/) { HandleRead(); });
  // we are always reading the wakeup fd
  wakeup_channel_->enableReading();
}

EventLoop::~EventLoop() {
  VLOG(1) << "EventLoop " << this << " of thread " << thread_id_
          << " destructs in thread " << this_thread::tid();
  wakeup_channel_->disableAll();
  wakeup_channel_->remove();
  ::close(wakeup_fd_);
  this_thread_event_loop = nullptr;
}

void EventLoop::Loop() {
  assert(!looping_);
  AssertInLoopThread();
  looping_ = true;
  VLOG(1) << "EventLoop " << this << " start looping";

  absl::MutexLock quit_lock(&quit_mutex_);
  while (!quit_) {
    quit_mutex_.Unlock();
    active_channels_.clear();
    absl::Time poll_return_time = poller_->poll(kPollTimeMs, &active_channels_);
    {
      absl::WriterMutexLock lock(&poll_return_time_mutex_);
      poll_return_time_ = poll_return_time;
    }
    iteration_.fetch_add(1, std::memory_order_acq_rel);

    if (VLOG_IS_ON(1)) {
      for (const Channel* channel : active_channels_) {
        VLOG(1) << "{" << channel->reventsToString() << "} ";
      }
    }

    // TODO(chenshuo): sort channel by priority

    handling_events_.store(true, std::memory_order_release);
    for (Channel* channel : active_channels_) {
      current_active_channel_ = channel;
      current_active_channel_->handleEvent(poll_return_time);
    }
    current_active_channel_ = nullptr;
    handling_events_.store(false, std::memory_order_release);

    InvokePendingFunctors();
    quit_mutex_.Lock();
  }

  VLOG(1) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::Quit() {
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  absl::MutexLock quit_lock(&quit_mutex_);
  quit_ = true;

  if (!IsInLoopThread()) {
    Wakeup();
  }
}

void EventLoop::RunInLoop(Functor cb) {
  if (IsInLoopThread()) {
    cb();
  } else {
    QueueInLoop(std::move(cb));
  }
}

void EventLoop::QueueInLoop(Functor cb) {
  {
    absl::MutexLock lock(&mutex_);
    pending_functors_.emplace_back(std::move(cb));
  }

  if (!IsInLoopThread() ||
      calling_pending_functors_.load(std::memory_order_acquire)) {
    Wakeup();
  }
}

size_t EventLoop::queue_size() const {
  absl::MutexLock lock(&mutex_);
  return pending_functors_.size();
}

TimerId EventLoop::RunAt(absl::Time time, TimerCallback cb) {
  return timer_queue_->addTimer(std::move(cb), time, absl::ZeroDuration());
}

TimerId EventLoop::RunAfter(absl::Duration delay, TimerCallback cb) {
  absl::Time time = absl::Now() + delay;
  return RunAt(time, std::move(cb));
}

TimerId EventLoop::RunEvery(absl::Duration interval, TimerCallback cb) {
  absl::Time time = absl::Now() + interval;
  return timer_queue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::CancelTimer(TimerId timer_id) {
  return timer_queue_->cancel(timer_id);
}

void EventLoop::UpdateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  AssertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  AssertInLoopThread();
  if (handling_events_.load(std::memory_order_acquire)) {
    assert(current_active_channel_ == channel ||
           std::find(active_channels_.begin(), active_channels_.end(),
                     channel) == active_channels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  AssertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::AbortIfNotInLoopThread() {
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
             << " was created in threadId_ = " << thread_id_
             << ", current thread id = " << this_thread::tid();
}

void EventLoop::Wakeup() {  // NOLINT(readability-make-member-function-const)
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeup_fd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::
    HandleRead() {  // NOLINT(readability-make-member-function-const)
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG(ERROR) << "EventLoop::HandleRead() reads " << n
               << " bytes instead of 8";
  }
}

void EventLoop::InvokePendingFunctors() {
  calling_pending_functors_.store(true, std::memory_order_release);

  std::vector<Functor> functors;
  {
    absl::MutexLock lock(&mutex_);
    // move & clear to preserve the capacity of `pending_functors_`.
    functors = std::move(pending_functors_);
    pending_functors_.clear();
  }

  for (const Functor& functor : functors) {
    functor();
  }

  calling_pending_functors_.store(false, std::memory_order_release);
}

}  // namespace net
}  // namespace jinduo
