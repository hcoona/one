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

  EventLoopThreadPool(EventLoop* base_loop, std::string name);
  ~EventLoopThreadPool();

  // Disallow copy.
  EventLoopThreadPool(const EventLoopThreadPool&) noexcept = delete;
  EventLoopThreadPool& operator=(const EventLoopThreadPool&) noexcept = delete;

  // Allow move.
  EventLoopThreadPool(EventLoopThreadPool&&) noexcept = default;
  EventLoopThreadPool& operator=(EventLoopThreadPool&&) noexcept = default;

  [[nodiscard]] bool started() const { return started_; }
  [[nodiscard]] const std::string& name() const { return name_; }

  // 0 to use `base_loop`, any positive numbers to establish a thread pool with
  // each thread a event loop.
  void set_thread_num(int num_threads) { num_threads_ = num_threads; }
  void Start(const ThreadInitCallback& cb = ThreadInitCallback());

  // Get next loop in a round-robin policy. Must call after `Start()`.
  EventLoop* GetNextLoop();

  /// Get next loop in a hashing policy. It will always return the same
  /// EventLoop for same hash_code.
  EventLoop* GetLoopForHash(size_t hash_code);

  std::vector<EventLoop*> all_loops();

 private:
  EventLoop* base_loop_;
  std::string name_;

  bool started_{false};
  int num_threads_{0};
  int next_loop_index_{0};

  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net
}  // namespace jinduo
