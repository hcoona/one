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

}  // namespace hcoona
