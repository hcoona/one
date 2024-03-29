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

#include "one/jinduo/net/event_loop_thread.h"

#include <pthread.h>

#include <utility>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"

namespace jinduo {
namespace net {

EventLoopThread::EventLoopThread(ThreadInitCallback cb, std::string name)
    : callback_(std::move(cb)), thread_name_(std::move(name)) {}

EventLoopThread::~EventLoopThread() {
  if (loop_ != nullptr) {  // not 100% race-free, eg. ThreadFunc could be
                           // running callback_.
    // still a tiny chance to call destructed object, if ThreadFunc exits just
    // now. but when EventLoopThread destructs, usually programming is exiting
    // anyway.
    loop_->Quit();
  }

  if (thread_) {
    thread_->join();
  }
}

EventLoop* EventLoopThread::StartLoop() {
  assert(!thread_);
  thread_ = std::make_optional<std::thread>(&EventLoopThread::ThreadFunc, this);

  EventLoop* loop = nullptr;
  {
    absl::MutexLock lock(&mutex_);
    while (loop_ == nullptr) {
      loop_initialized_cv_.Wait(&mutex_);
    }
    loop = loop_;
  }

  return loop;
}

void EventLoopThread::ThreadFunc() {
  static constexpr size_t kThreadNameLengthMax = 15;
  PLOG_IF(WARNING,
          pthread_setname_np(
              thread_->native_handle(),
              thread_name_.size() < kThreadNameLengthMax
                  ? thread_name_.c_str()
                  : std::string(thread_name_.begin(),
                                thread_name_.begin() + kThreadNameLengthMax)
                        .c_str()) != 0)
      << "Failed to set event loop thread name. name=" << thread_name_;

  EventLoop loop;

  if (callback_) {
    callback_(&loop);
  }

  {
    absl::MutexLock lock(&mutex_);
    loop_ = &loop;
    loop_initialized_cv_.Signal();
  }

  loop.Loop();

  {
    absl::MutexLock lock(&mutex_);
    loop_ = nullptr;
  }
}

}  // namespace net
}  // namespace jinduo
