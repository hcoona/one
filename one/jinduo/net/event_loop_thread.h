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

#include <functional>
#include <optional>
#include <string>
#include <thread>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

namespace jinduo {
namespace net {

class EventLoop;

class EventLoopThread {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  explicit EventLoopThread(ThreadInitCallback cb = ThreadInitCallback(),
                           std::string name = std::string());
  ~EventLoopThread();

  // Disallow copy.
  EventLoopThread(const EventLoopThread&) noexcept = delete;
  EventLoopThread& operator=(const EventLoopThread&) noexcept = delete;

  // Allow move but not implemented yet.
  EventLoopThread(EventLoopThread&&) noexcept = delete;
  EventLoopThread& operator=(EventLoopThread&&) noexcept = delete;

  EventLoop* startLoop();

 private:
  void threadFunc();

  absl::Mutex mutex_{};
  absl::CondVar cond_ ABSL_GUARDED_BY(mutex_){};
  EventLoop* loop_ ABSL_GUARDED_BY(mutex_){nullptr};
  bool exiting_{false};

  ThreadInitCallback callback_{};
  std::optional<std::thread> thread_{};
  std::string thread_name_{};
};

}  // namespace net
}  // namespace jinduo
