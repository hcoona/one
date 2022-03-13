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

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "absl/synchronization/blocking_counter.h"

namespace jinduo {

class Thread {
 public:
  using ThreadFunc = std::function<void()>;

  explicit Thread(ThreadFunc, std::string name = std::string());
  // FIXME: make it movable in C++11
  ~Thread();

  // Disallow copy.
  Thread(const Thread&) noexcept = delete;
  Thread& operator=(const Thread&) noexcept = delete;

  // Allow move but not implemented yet.
  Thread(Thread&&) noexcept = delete;
  Thread& operator=(Thread&&) noexcept = delete;

  void start();
  int join();  // return pthread_join()

  [[nodiscard]] bool started() const { return started_; }
  // pthread_t pthreadId() const { return pthreadId_; }
  [[nodiscard]] int tid() const { return tid_; }
  [[nodiscard]] const std::string& name() const { return name_; }

  static int numCreated() {
    return numCreated_.load(std::memory_order_acquire);
  }

 private:
  void setDefaultName();

  bool started_{false};
  bool joined_{false};
  unsigned long pthreadId_{0};  // NOLINT: unsigned long for pthread_t
  int tid_{0};
  ThreadFunc func_{};
  std::string name_{};
  absl::BlockingCounter latch_{1};

  static std::atomic<int32_t> numCreated_;
};

}  // namespace jinduo
