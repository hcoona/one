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
#include "absl/strings/str_cat.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/event_loop_thread.h"

namespace jinduo {
namespace net {

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, std::string name)
    : base_loop_(base_loop), name_(std::move(name)) {}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::Start(const ThreadInitCallback& cb) {
  assert(!started_);
  base_loop_->AssertInLoopThread();

  started_ = true;

  for (int i = 0; i < num_threads_; ++i) {
    auto* t = new EventLoopThread(cb, absl::StrCat(name_, i));
    threads_.emplace_back(t);
    loops_.emplace_back(t->StartLoop());
  }

  if (num_threads_ == 0 && cb) {
    cb(base_loop_);
  }
}

EventLoop* EventLoopThreadPool::GetNextLoop() {
  base_loop_->AssertInLoopThread();
  assert(started_);
  EventLoop* loop = base_loop_;

  if (!loops_.empty()) {
    // round-robin
    loop = loops_[next_loop_index_];
    ++next_loop_index_;
    if (absl::implicit_cast<size_t>(next_loop_index_) >= loops_.size()) {
      next_loop_index_ = 0;
    }
  }
  return loop;
}

EventLoop* EventLoopThreadPool::GetLoopForHash(size_t hash_code) {
  base_loop_->AssertInLoopThread();
  EventLoop* loop = base_loop_;

  if (!loops_.empty()) {
    loop = loops_[hash_code % loops_.size()];
  }

  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::all_loops() {
  base_loop_->AssertInLoopThread();
  assert(started_);
  if (loops_.empty()) {
    return {base_loop_};
  }
  return loops_;
}

}  // namespace net
}  // namespace jinduo
