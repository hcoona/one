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
// This is an public header file.

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace jinduo {
namespace net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop* baseLoop, std::string nameArg);
  ~EventLoopThreadPool();

  // Disallow copy.
  EventLoopThreadPool(const EventLoopThreadPool&) noexcept = delete;
  EventLoopThreadPool& operator=(const EventLoopThreadPool&) noexcept = delete;

  // Allow move but not implemented yet.
  EventLoopThreadPool(EventLoopThreadPool&&) noexcept = delete;
  EventLoopThreadPool& operator=(EventLoopThreadPool&&) noexcept = delete;

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop* getNextLoop();

  /// with the same hash code, it will always return the same EventLoop
  EventLoop* getLoopForHash(size_t hashCode);

  std::vector<EventLoop*> getAllLoops();

  [[nodiscard]] bool started() const { return started_; }

  [[nodiscard]] const std::string& name() const { return name_; }

 private:
  EventLoop* baseLoop_;
  std::string name_;
  bool started_{false};
  int numThreads_{0};
  int next_{0};
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net
}  // namespace jinduo
