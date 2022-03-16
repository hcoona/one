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

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>
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

__thread EventLoop* t_loopInThisThread = 0;

const int kPollTimeMs = 10000;

int createEventfd() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG(ERROR) << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { ::signal(SIGPIPE, SIG_IGN); }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;

}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread() {
  return t_loopInThisThread;
}

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      eventHandling_(false),
      callingPendingFunctors_(false),
      iteration_(0),
      threadId_(this_thread::tid()),
      poller_(Poller::newDefaultPoller(this)),
      timerQueue_(new TimerQueue(this)),
      wakeupFd_(createEventfd()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr) {
  VLOG(1) << "EventLoop created " << this << " in thread " << threadId_;
  if (t_loopInThisThread != nullptr) {
    LOG(FATAL) << "Another EventLoop " << t_loopInThisThread
               << " exists in this thread " << threadId_;
  } else {
    t_loopInThisThread = this;
  }
  wakeupChannel_->setReadCallback(std::bind(  // NOLINT(modernize-avoid-bind):
                                              // By-design ignore timestamp arg
      &EventLoop::handleRead, this));
  // we are always reading the wakeupfd
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  VLOG(1) << "EventLoop " << this << " of thread " << threadId_
          << " destructs in thread " << this_thread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
  VLOG(1) << "EventLoop " << this << " start looping";

  while (!quit_) {
    activeChannels_.clear();
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);
    ++iteration_;
    if (VLOG_IS_ON(1)) {
      printActiveChannels();
    }
    // TODO(chenshuo): sort channel by priority
    eventHandling_ = true;
    for (Channel* channel : activeChannels_) {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }
    currentActiveChannel_ = nullptr;
    eventHandling_ = false;
    doPendingFunctors();
  }

  VLOG(1) << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor cb) {
  {
    absl::MutexLock lock(&mutex_);
    pendingFunctors_.push_back(std::move(cb));
  }

  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

size_t EventLoop::queueSize() const {
  absl::MutexLock lock(&mutex_);
  return pendingFunctors_.size();
}

TimerId EventLoop::runAt(absl::Time time, TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb), time, absl::ZeroDuration());
}

TimerId EventLoop::runAfter(absl::Duration delay, TimerCallback cb) {
  absl::Time time = absl::Now() + delay;
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(absl::Duration interval, TimerCallback cb) {
  absl::Time time = absl::Now() + interval;
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId) { return timerQueue_->cancel(timerId); }

void EventLoop::updateChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_) {
    assert(currentActiveChannel_ == channel ||
           std::find(activeChannels_.begin(), activeChannels_.end(), channel) ==
               activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread() {
  LOG(FATAL) << "EventLoop::abortNotInLoopThread - EventLoop " << this
             << " was created in threadId_ = " << threadId_
             << ", current thread id = " << this_thread::tid();
}

void EventLoop::wakeup() {  // NOLINT(readability-make-member-function-const)
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::
    handleRead() {  // NOLINT(readability-make-member-function-const)
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one) {
    LOG(ERROR) << "EventLoop::handleRead() reads " << n
               << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    absl::MutexLock lock(&mutex_);
    functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors) {
    functor();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const {
  for (const Channel* channel : activeChannels_) {
    VLOG(1) << "{" << channel->reventsToString() << "} ";
  }
}

}  // namespace net
}  // namespace jinduo
