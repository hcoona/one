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
// This is a public header file, it must only include public header files.

#pragma once

#include <any>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "one/jinduo/base/this_thread.h"
#include "one/jinduo/net/callbacks.h"
#include "one/jinduo/net/timer_id.h"

namespace jinduo {
namespace net {

class Channel;
class Poller;
class TimerQueue;

// Reactor, at most one per thread.
//
// This is an interface class, so don't expose too much details.
class EventLoop {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();  // force out-line dtor, for std::unique_ptr members.

  // Disallow copy.
  EventLoop(const EventLoop&) noexcept = delete;
  EventLoop& operator=(const EventLoop&) noexcept = delete;

  // Allow move but not implemented yet.
  EventLoop(EventLoop&&) noexcept = delete;
  EventLoop& operator=(EventLoop&&) noexcept = delete;

  static EventLoop* GetCurrentThreadEventLoop();

  // Loops forever.
  //
  // Must be called in the same thread as creation of the object.
  void Loop();

  // Quits loop.
  //
  // This is not 100% thread safe if you call through a raw pointer.
  // It's better to call through shared_ptr<EventLoop> for 100% safety.
  void Quit();

  // Time when poll returns, usually means data arrival.
  absl::Time PollReturnTime() const { return poll_return_time_; }

  int64_t iteration() const { return iteration_; }

  // Runs callback immediately in the loop thread.
  // It wakes up the loop, and run the cb.
  // If in the same loop thread, cb is run within the function.
  // Safe to call from other threads.
  void RunInLoop(Functor cb);

  // Queues callback in the loop thread.
  // Runs after finish pooling.
  // Safe to call from other threads.
  void QueueInLoop(Functor cb);

  size_t QueueSize() const;

  //
  // timers
  //

  // Runs callback at 'time'.
  // Safe to call from other threads.
  TimerId RunAt(absl::Time time, TimerCallback cb);
  // Runs callback after @c delay seconds.
  // Safe to call from other threads.
  TimerId RunAfter(absl::Duration delay, TimerCallback cb);
  // Runs callback every @c interval seconds.
  // Safe to call from other threads.
  TimerId RunEvery(absl::Duration interval, TimerCallback cb);
  // Cancels the timer.
  // Safe to call from other threads.
  void CancelTimer(TimerId timer_id);

  // pid_t threadId() const { return threadId_; }
  void AssertInLoopThread() {
    if (!IsInLoopThread()) {
      AbortIfNotInLoopThread();
    }
  }
  bool IsInLoopThread() const { return thread_id_ == this_thread::tid(); }
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool event_handling() const { return event_handling_; }

  void set_context(const std::any& context) { context_ = context; }

  const std::any& context() const { return context_; }

  std::any* mutable_context() { return &context_; }

  //
  // internal usage
  //

  void wakeup();
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

 private:
  void AbortIfNotInLoopThread();
  void HandleRead();  // waked up
  void InvokePendingFunctors();

  void PrintActiveChannels() const;  // DEBUG

  using ChannelList = std::vector<Channel*>;

  bool looping_; /* atomic */
  std::atomic<bool> quit_;
  bool event_handling_;           /* atomic */
  bool calling_pending_functors_; /* atomic */
  int64_t iteration_;
  const int thread_id_;
  absl::Time poll_return_time_;
  std::unique_ptr<Poller> poller_;
  std::unique_ptr<TimerQueue> timer_queue_;
  int wakeup_fd_;
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  std::unique_ptr<Channel> wakeup_channel_;
  std::any context_;

  // scratch variables
  ChannelList active_channels_;
  Channel* current_active_channel_;

  mutable absl::Mutex mutex_;
  std::vector<Functor> pending_functors_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace net
}  // namespace jinduo
