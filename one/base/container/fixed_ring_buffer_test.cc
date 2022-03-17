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

#include "one/base/container/fixed_ring_buffer.h"

#include <cstdint>
#include <vector>

#include "gtest/gtest.h"

namespace hcoona {

namespace {

class MoveOnlyInt {
 public:
  MoveOnlyInt() = default;
  constexpr explicit MoveOnlyInt(int value) : value_(value) {}
  ~MoveOnlyInt() = default;

  // Disallow copy.
  MoveOnlyInt(const MoveOnlyInt&) = delete;
  MoveOnlyInt& operator=(const MoveOnlyInt&) = delete;

  // Allow move.
  constexpr MoveOnlyInt(MoveOnlyInt&&) noexcept = default;
  MoveOnlyInt& operator=(MoveOnlyInt&&) noexcept = default;

  explicit operator int() const { return value_; }

 private:
  int value_;
};

}  // namespace

TEST(FixedRingBuffer, ConstructTrivial) {
  FixedRingBuffer<int32_t, 3> ring_buffer;
}

TEST(FixedRingBuffer, ConstructN) {
  FixedRingBuffer<int32_t, 3> ring_buffer(static_cast<size_t>(2));
  ASSERT_EQ(2, ring_buffer.size());
}

TEST(FixedRingBuffer, ConstructNWithValue) {
  FixedRingBuffer<int32_t, 3> ring_buffer(static_cast<size_t>(2), 1);
  ASSERT_EQ(2, ring_buffer.size());
  EXPECT_EQ(1, ring_buffer[0]);
  EXPECT_EQ(1, ring_buffer[1]);
}

TEST(FixedRingBuffer, ConstructIlist) {
  FixedRingBuffer<int32_t, 3> ring_buffer{1, 2, 3};
  ASSERT_EQ(3, ring_buffer.size());
  EXPECT_EQ(1, ring_buffer[0]);
  EXPECT_EQ(2, ring_buffer[1]);
  EXPECT_EQ(3, ring_buffer[2]);
}

TEST(FixedRingBuffer, Copy) {
  FixedRingBuffer<int32_t, 3> ring_buffer{1, 2, 3};
  ASSERT_EQ(3, ring_buffer.size());
  EXPECT_EQ(1, ring_buffer[0]);
  EXPECT_EQ(2, ring_buffer[1]);
  EXPECT_EQ(3, ring_buffer[2]);

  FixedRingBuffer<int32_t, 3> ring_buffer_other(ring_buffer);
  EXPECT_EQ(ring_buffer, ring_buffer_other);
}

TEST(FixedRingBuffer, EmplaceBack) {
  FixedRingBuffer<MoveOnlyInt, 3> ring_buffer;
  ring_buffer.emplace_back(1);
  ring_buffer.emplace_back(2);
  ring_buffer.emplace_back(3);
  ASSERT_EQ(3, ring_buffer.size());
  EXPECT_EQ(1, static_cast<int>(ring_buffer[0]));
  EXPECT_EQ(2, static_cast<int>(ring_buffer[1]));
  EXPECT_EQ(3, static_cast<int>(ring_buffer[2]));
}

TEST(FixedRingBuffer, EmplaceFront) {
  FixedRingBuffer<MoveOnlyInt, 3> ring_buffer;
  ring_buffer.emplace_front(3);
  ring_buffer.emplace_front(2);
  ring_buffer.emplace_front(1);
  ASSERT_EQ(3, ring_buffer.size());
  EXPECT_EQ(1, static_cast<int>(ring_buffer[0]));
  EXPECT_EQ(2, static_cast<int>(ring_buffer[1]));
  EXPECT_EQ(3, static_cast<int>(ring_buffer[2]));
}

TEST(FixedRingBuffer, Move) {
  FixedRingBuffer<MoveOnlyInt, 3> ring_buffer;
  ring_buffer.emplace_back(1);
  ring_buffer.emplace_back(2);
  ring_buffer.emplace_back(3);
  ASSERT_EQ(3, ring_buffer.size());
  EXPECT_EQ(1, static_cast<int>(ring_buffer[0]));
  EXPECT_EQ(2, static_cast<int>(ring_buffer[1]));
  EXPECT_EQ(3, static_cast<int>(ring_buffer[2]));

  FixedRingBuffer<MoveOnlyInt, 3> ring_buffer_other(std::move(ring_buffer));
  ASSERT_EQ(3, ring_buffer_other.size());
  EXPECT_EQ(1, static_cast<int>(ring_buffer_other[0]));
  EXPECT_EQ(2, static_cast<int>(ring_buffer_other[1]));
  EXPECT_EQ(3, static_cast<int>(ring_buffer_other[2]));
}

TEST(FixedRingBuffer, RangeFor) {
  FixedRingBuffer<int, 3> ring_buffer{1, 2, 3};
  ASSERT_EQ(3, ring_buffer.size());

  int count = 0;
  for (int v : ring_buffer) {
    EXPECT_EQ(count + 1, v);
    count++;
  }
}

TEST(FixedRingBuffer, EmptyHit) {
  FixedRingBuffer<int, 3> ring_buffer;
  EXPECT_TRUE(ring_buffer.empty());
}

TEST(FixedRingBuffer, EmptyMiss) {
  FixedRingBuffer<int, 3> ring_buffer(1);
  EXPECT_FALSE(ring_buffer.empty());
}

TEST(FixedRingBuffer, FullHit) {
  FixedRingBuffer<int, 3> ring_buffer(3);
  EXPECT_TRUE(ring_buffer.full());
}

TEST(FixedRingBuffer, FullMiss) {
  FixedRingBuffer<int, 3> ring_buffer(1);
  EXPECT_FALSE(ring_buffer.full());
}

TEST(FixedRingBuffer, Clear) {
  FixedRingBuffer<int, 3> ring_buffer(3);
  ring_buffer.clear();
  EXPECT_TRUE(ring_buffer.empty());
}

TEST(FixedRingBuffer, Front) {
  FixedRingBuffer<int, 3> ring_buffer{1, 2, 3};
  EXPECT_EQ(1, ring_buffer.front());
}

TEST(FixedRingBuffer, Back) {
  FixedRingBuffer<int, 3> ring_buffer{1, 2, 3};
  EXPECT_EQ(3, ring_buffer.back());
}

TEST(FixedRingBuffer, PopFront) {
  FixedRingBuffer<int, 3> ring_buffer{1, 2, 3};
  ring_buffer.pop_front();
  EXPECT_EQ(2, static_cast<int>(ring_buffer[0]));
  EXPECT_EQ(3, static_cast<int>(ring_buffer[1]));
}

TEST(FixedRingBuffer, PopBack) {
  FixedRingBuffer<int, 3> ring_buffer{1, 2, 3};
  ring_buffer.pop_back();
  EXPECT_EQ(1, static_cast<int>(ring_buffer[0]));
  EXPECT_EQ(2, static_cast<int>(ring_buffer[1]));
}

}  // namespace hcoona
