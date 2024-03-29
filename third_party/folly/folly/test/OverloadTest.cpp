/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "folly/Overload.h"

#include <optional>
#include <variant>

#include "boost/variant.hpp"

#include "folly/DiscriminatedPtr.h"
#include "folly/portability/GTest.h"

namespace folly {
namespace test {

struct One {
  std::string toString() const { return "One"; }
};
struct Two {
  std::string toString() const { return "Two"; }
};

TEST(Overload, StdVariant) {
  using V = std::variant<One, Two>;
  V one(One{});
  V two(Two{});

  EXPECT_TRUE(variant_match(
      one, [](const One&) { return true; }, [](const Two&) { return false; }));
  EXPECT_TRUE(variant_match(
      two, [](const One&) { return false; }, [](const Two&) { return true; }));

  auto toString = [](const auto& variant) {
    return variant_match(
        variant, [](const auto& value) { return value.toString(); });
  };
  EXPECT_EQ(toString(one), "One");
  EXPECT_EQ(toString(two), "Two");
}

TEST(Overload, BoostVariant) {
  using V = boost::variant<One, Two>;
  V one(One{});
  V two(Two{});

  EXPECT_TRUE(variant_match(
      one, [](const One&) { return true; }, [](const Two&) { return false; }));
  EXPECT_TRUE(variant_match(
      two, [](const One&) { return false; }, [](const Two&) { return true; }));

  auto toString = [](const auto& variant) {
    return variant_match(
        variant, [](const auto& value) { return value.toString(); });
  };
  EXPECT_EQ(toString(one), "One");
  EXPECT_EQ(toString(two), "Two");
}

TEST(Overload, DiscriminatedPtr) {
  using V = DiscriminatedPtr<One, Two>;
  One one_obj;
  Two two_obj;
  V one_ptr(&one_obj);
  V two_ptr(&two_obj);

  EXPECT_TRUE(variant_match(
      one_ptr,
      [](const One*) { return true; },
      [](const Two*) { return false; }));
  EXPECT_TRUE(variant_match(
      two_ptr,
      [](const One*) { return false; },
      [](const Two*) { return true; }));

  auto toString = [](const auto& variant) {
    return variant_match(
        variant, [](const auto* value) { return value->toString(); });
  };
  EXPECT_EQ(toString(one_ptr), "One");
  EXPECT_EQ(toString(two_ptr), "Two");
}

TEST(Overload, StdPattern) {
  using V = std::variant<One, Two>;
  V one(One{});
  V two(Two{});

  auto is_one_overload = overload(
      [](const One&) { return true; }, [](const Two&) { return false; });
  EXPECT_TRUE(std::visit(is_one_overload, one));
  EXPECT_TRUE(variant_match(one, is_one_overload));
  EXPECT_FALSE(variant_match(two, is_one_overload));

  auto is_two_overload = overload(
      [](const One&) { return false; }, [](const Two&) { return true; });
  EXPECT_TRUE(std::visit(is_two_overload, two));
  EXPECT_FALSE(variant_match(one, is_two_overload));
  EXPECT_TRUE(variant_match(two, is_two_overload));

  auto is_one_copy = overload(is_one_overload);
  auto is_one_const_copy =
      overload(static_cast<const decltype(is_one_overload)&>(is_one_overload));
  EXPECT_TRUE(variant_match(one, is_one_copy));
  EXPECT_TRUE(variant_match(one, is_one_const_copy));
  EXPECT_FALSE(variant_match(two, is_one_copy));
  EXPECT_FALSE(variant_match(two, is_one_const_copy));
}

TEST(Overload, BoostPattern) {
  using V = boost::variant<One, Two>;
  V one(One{});
  V two(Two{});

  auto is_one_overload = overload(
      [](const One&) { return true; }, [](const Two&) { return false; });
  EXPECT_TRUE(boost::apply_visitor(is_one_overload, one));
  EXPECT_TRUE(variant_match(one, is_one_overload));
  EXPECT_FALSE(variant_match(two, is_one_overload));

  auto is_two_overload = overload(
      [](const One&) { return false; }, [](const Two&) { return true; });
  EXPECT_TRUE(boost::apply_visitor(is_two_overload, two));
  EXPECT_FALSE(variant_match(one, is_two_overload));
  EXPECT_TRUE(variant_match(two, is_two_overload));

  auto is_one_copy = overload(is_one_overload);
  auto is_one_const_copy =
      overload(static_cast<const decltype(is_one_overload)&>(is_one_overload));
  EXPECT_TRUE(variant_match(one, is_one_copy));
  EXPECT_TRUE(variant_match(one, is_one_const_copy));
  EXPECT_FALSE(variant_match(two, is_one_copy));
  EXPECT_FALSE(variant_match(two, is_one_const_copy));
}

TEST(Overload, ReturnType) {
  using V = std::variant<std::monostate, One>;
  V null(std::monostate{});
  V one(One{});

  using R = std::optional<int>;

  EXPECT_FALSE(variant_match(
      null,
      [](const std::monostate&) -> R { return std::nullopt; },
      [](const One&) -> R { return 1; }));
  EXPECT_EQ(
      1,
      variant_match(
          one,
          [](const std::monostate&) -> R { return std::nullopt; },
          [](const One&) -> R { return 1; }));

  EXPECT_FALSE(variant_match<R>(
      null,
      [](const std::monostate&) { return std::nullopt; },
      [](const One&) { return 1; }));
  EXPECT_EQ(
      1,
      variant_match<R>(
          one,
          [](const std::monostate&) { return std::nullopt; },
          [](const One&) { return 1; }));

  int null_result = -1;
  variant_match<void>(
      null,
      [&null_result](const std::monostate&) {
        null_result = 0;
        return std::nullopt;
      },
      [&null_result](const One&) {
        null_result = 1;
        return 1;
      });
  EXPECT_EQ(0, null_result);

  int one_result = -1;
  variant_match<void>(
      one,
      [&one_result](const std::monostate&) {
        one_result = 0;
        return std::nullopt;
      },
      [&one_result](const One&) {
        one_result = 1;
        return 1;
      });
  EXPECT_EQ(1, one_result);
}

} // namespace test
} // namespace folly
