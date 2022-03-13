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
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/base/thread.h"

#include <errno.h>
#include <linux/unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <type_traits>
#include <utility>

#include "glog/logging.h"
#include "one/jinduo/base/this_thread.h"

namespace jinduo {
namespace detail {

namespace {

void afterFork() {
  jinduo::this_thread::details::t_cachedTid = 0;
  jinduo::this_thread::details::t_threadName = "main";
  jinduo::this_thread::tid();
  // no need to call pthread_atfork(NULL, NULL, &afterFork);
}

}  // namespace

class ThreadNameInitializer {
 public:
  ThreadNameInitializer() {
    jinduo::this_thread::details::t_threadName = "main";
    jinduo::this_thread::tid();
    pthread_atfork(nullptr, nullptr, &afterFork);
  }
};

ThreadNameInitializer init;

struct ThreadData {
  using ThreadFunc = jinduo::Thread::ThreadFunc;
  ThreadFunc func_;
  std::string name_;
  pid_t* tid_;
  absl::BlockingCounter* latch_;

  ThreadData(ThreadFunc func, std::string name, pid_t* tid,
             absl::BlockingCounter* latch)
      : func_(std::move(func)),
        name_(std::move(name)),
        tid_(tid),
        latch_(latch) {}

  void runInThread() {
    *tid_ = jinduo::this_thread::tid();
    tid_ = nullptr;
    latch_->DecrementCount();
    latch_ = nullptr;

    jinduo::this_thread::details::t_threadName =
        name_.empty() ? "muduoThread" : name_.c_str();
    ::prctl(PR_SET_NAME, jinduo::this_thread::details::t_threadName);
    try {
      func_();
      jinduo::this_thread::details::t_threadName = "finished";
    } catch (const std::exception& ex) {
      jinduo::this_thread::details::t_threadName = "crashed";
      fprintf(stderr, "exception caught in Thread %s\n", name_.c_str());
      fprintf(stderr, "reason: %s\n", ex.what());
      abort();
    } catch (...) {
      jinduo::this_thread::details::t_threadName = "crashed";
      fprintf(stderr, "unknown exception caught in Thread %s\n", name_.c_str());
      throw;  // rethrow
    }
  }
};

void* startThread(void* obj) {
  auto* data = reinterpret_cast<ThreadData*>(obj);
  data->runInThread();
  delete data;
  return nullptr;
}

}  // namespace detail

std::atomic<int32_t> Thread::numCreated_;

Thread::Thread(ThreadFunc func, std::string name)
    : func_(std::move(func)), name_(std::move(name)) {
  setDefaultName();
}

Thread::~Thread() {
  if (started_ && !joined_) {
    pthread_detach(pthreadId_);
  }
}

void Thread::setDefaultName() {
  int num = numCreated_.fetch_add(1, std::memory_order_acq_rel) + 1;
  if (name_.empty()) {
    static constexpr size_t kBufferSize = 32;
    char buf[kBufferSize];
    snprintf(buf, sizeof(buf), "Thread%d", num);
    name_ = buf;
  }
}

void Thread::start() {
  assert(!started_);
  started_ = true;
  // FIXME: move(func_)
  auto* data = new detail::ThreadData(func_, name_, &tid_, &latch_);
  static_assert(sizeof(pthread_t) == sizeof(pthreadId_),
                "pid_t type size mismatch.");
  if (::pthread_create(&pthreadId_, nullptr, &detail::startThread, data) != 0) {
    started_ = false;
    delete data;  // or no delete?
    LOG(FATAL) << "Failed in pthread_create";
  } else {
    latch_.Wait();
    assert(tid_ > 0);
  }
}

int Thread::join() {
  assert(started_);
  assert(!joined_);
  joined_ = true;
  return pthread_join(pthreadId_, nullptr);
}

}  // namespace jinduo
