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

#include "one/jinduo/net/event_loop_thread_pool.h"

#include <stdio.h>

#include <memory>
#include <utility>
#include <vector>

#include "absl/base/casts.h"
#include "absl/container/fixed_array.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/event_loop_thread.h"

namespace jinduo {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         std::string nameArg)
    : baseLoop_(baseLoop), name_(std::move(nameArg)) {}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  assert(!started_);
  baseLoop_->AssertInLoopThread();

  started_ = true;

  for (int i = 0; i < numThreads_; ++i) {
    absl::FixedArray<char> buf(name_.size() + 32);
    snprintf(buf.data(), buf.size(), "%s%d", name_.c_str(), i);
    auto* t = new EventLoopThread(cb, buf.data());
    threads_.emplace_back(t);
    loops_.emplace_back(t->StartLoop());
  }
  if (numThreads_ == 0 && cb) {
    cb(baseLoop_);
  }
}

EventLoop* EventLoopThreadPool::getNextLoop() {
  baseLoop_->AssertInLoopThread();
  assert(started_);
  EventLoop* loop = baseLoop_;

  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_];
    ++next_;
    if (absl::implicit_cast<size_t>(next_) >= loops_.size()) {
      next_ = 0;
    }
  }
  return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
  baseLoop_->AssertInLoopThread();
  EventLoop* loop = baseLoop_;

  if (!loops_.empty()) {
    loop = loops_[hashCode % loops_.size()];
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
  baseLoop_->AssertInLoopThread();
  assert(started_);
  if (loops_.empty()) {
    return {baseLoop_};
  }
  return loops_;
}

}  // namespace net
}  // namespace jinduo
