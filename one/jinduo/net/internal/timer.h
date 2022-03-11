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
// This is an internal header file, you should not include this.

#pragma once

#include <atomic>
#include <cstddef>
#include <utility>

#include "absl/time/time.h"
#include "one/jinduo/net/callbacks.h"

namespace jinduo {
namespace net {

// Internal class for timer event.
class Timer {
 public:
  Timer(TimerCallback cb, absl::Time when, absl::Duration interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > absl::ZeroDuration()),
        sequence_(s_numCreated_.fetch_add(1, std::memory_order_acq_rel) + 1) {}

  void run() const { callback_(); }

  [[nodiscard]] absl::Time expiration() const { return expiration_; }
  [[nodiscard]] bool repeat() const { return repeat_; }
  [[nodiscard]] int64_t sequence() const { return sequence_; }

  void restart(absl::Time now);

  static int64_t numCreated() {
    return s_numCreated_.load(std::memory_order_acquire);
  }

 private:
  const TimerCallback callback_;
  absl::Time expiration_;
  const absl::Duration interval_;
  const bool repeat_;
  const int64_t sequence_;

  static std::atomic<int64_t> s_numCreated_;
};

}  // namespace net
}  // namespace jinduo
