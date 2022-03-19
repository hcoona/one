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

#include <any>
#include <functional>
#include <memory>
#include <utility>

#include "absl/synchronization/notification.h"
#include "gtest/gtest.h"

namespace hcoona {

namespace {

template <typename F>
class Finally {
 public:
  explicit Finally(F&& f) : f_(std::forward<F>(f)) {}
  ~Finally() { f_(); }

  // Disallow copy.
  Finally(const Finally&) = delete;
  Finally& operator=(const Finally&) = delete;

  // Allow move.
  Finally(Finally&&) noexcept = default;
  Finally& operator=(Finally&&) noexcept = default;

 private:
  F f_;
};

template <typename F>
Finally<F> make_finally(F&& f) {
  return Finally<F>(std::forward<F>(f));
}

}  // namespace

TEST(TestStoreSharedPtrInAny, Case01) {
  static constexpr absl::Duration kTimeoutLimit = absl::Milliseconds(10);
  absl::Notification notification;

  {
    auto finally = std::make_shared<Finally<std::function<void()>>>(
        [&notification] { notification.Notify(); });
    std::any v = std::move(finally);
    ASSERT_FALSE(notification.WaitForNotificationWithTimeout(kTimeoutLimit));
  }

  EXPECT_TRUE(notification.WaitForNotificationWithTimeout(kTimeoutLimit));
}

}  // namespace hcoona
