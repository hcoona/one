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

#include "one/base/container/blocking_bounded_queue.h"

#include <optional>

#include "gtest/gtest.h"
#include "one/test/move_only_value.h"

namespace hcoona {

TEST(BlockingBoundedQueue, MoveOnlyValue) {
  BlockingBoundedQueue<MoveOnlyValue<int>, 3> queue;
  queue.emplace_back(1);
  MoveOnlyValue<int> v = queue.PopFrontAndReturn();
  EXPECT_EQ(1, v.value());
}

TEST(BlockingBoundedQueue, TryPopFrontEmpty) {
  BlockingBoundedQueue<MoveOnlyValue<int>, 3> queue;
  std::optional<MoveOnlyValue<int>> v = queue.TryPopFrontAndReturn();
  EXPECT_FALSE(v.has_value());
}

TEST(BlockingBoundedQueue, TryPopFrontNonEmpty) {
  BlockingBoundedQueue<MoveOnlyValue<int>, 3> queue;
  queue.emplace_back(1);
  std::optional<MoveOnlyValue<int>> v = queue.TryPopFrontAndReturn();
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(1, v->value());
}

TEST(BlockingBoundedQueue, TryPopBackEmpty) {
  BlockingBoundedQueue<MoveOnlyValue<int>, 3> queue;
  std::optional<MoveOnlyValue<int>> v = queue.TryPopBackAndReturn();
  EXPECT_FALSE(v.has_value());
}

TEST(BlockingBoundedQueue, TryPopBackNonEmpty) {
  BlockingBoundedQueue<MoveOnlyValue<int>, 3> queue;
  queue.emplace_back(1);
  std::optional<MoveOnlyValue<int>> v = queue.TryPopBackAndReturn();
  ASSERT_TRUE(v.has_value());
  EXPECT_EQ(1, v->value());
}

}  // namespace hcoona
