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
// Copied from https://github.com/microsoft/GSL/blob/v4.0.0/tests/span_tests.cpp
// Modifications:
// 1. Re-format with Google style.
// 2. Remove GSL dependencies.
// 3. Rename namespace `gsl` to `hcoona`.
// 4. Rename macro prefix `ONE_` to `ONE_`.
// 5. Inline `HAS_STRING_VIEW`.
// 6. Replace EXPECT_TRUE(\(.*) == (\d+\)); -> EXPECT_EQ\1, $2;
// 7. Suppress linter complaints.
// 8. Remove TEST(span_test, std_container_ctad).
// 9. Make `deathstring` an empty string because our hardening won't call
//    `std::terminate_handler`.

#include "one/base/container/span.h"

#include <array>     // for array
#include <cstddef>   // for byte, ptrdiff_t
#include <iostream>  // for ptrdiff_t
#include <iterator>  // for reverse_iterator, operator-, operator==
#include <memory>    // for unique_ptr, shared_ptr, make_unique, allo...
#include <regex>     // for match_results, sub_match, match_results<>...
#include <string>    // for string
#include <string_view>
#include <type_traits>  // for integral_constant<>::value, is_default_co...
#include <utility>
#include <vector>  // for vector

#include "gtest/gtest.h"

namespace {

constexpr char deathstring[] = "";
constexpr char failed_set_terminate_deathstring[] = ".*";

// This prevents a failed call to set_terminate from failing the test suite.
constexpr const char* GetExpectedDeathString(std::terminate_handler handle) {
  return handle != nullptr ? deathstring : failed_set_terminate_deathstring;
}

struct BaseClass {};
struct DerivedClass : BaseClass {};
struct AddressOverloaded {
#if (__cplusplus > 201402L)
  [[maybe_unused]]
#endif
  AddressOverloaded    // NOLINT(google-runtime-operator)
  operator&() const {  // NOLINT(runtime/operator)
    return {};
  }
};

}  // namespace

TEST(span_test, constructors) {
  hcoona::span<int> s;
  EXPECT_EQ(s.size(), 0);
  EXPECT_TRUE(s.data() == nullptr);

  hcoona::span<const int> cs;
  EXPECT_EQ(cs.size(), 0);
  EXPECT_TRUE(cs.data() == nullptr);
}

TEST(span_test, constructors_with_extent) {
  hcoona::span<int, 0> s;
  EXPECT_EQ(s.size(), 0);
  EXPECT_TRUE(s.data() == nullptr);

  hcoona::span<const int, 0> cs;
  EXPECT_EQ(cs.size(), 0);
  EXPECT_TRUE(cs.data() == nullptr);
}

TEST(span_test, constructors_with_bracket_init) {
  hcoona::span<int> s{};
  EXPECT_EQ(s.size(), 0);
  EXPECT_TRUE(s.data() == nullptr);

  hcoona::span<const int> cs{};
  EXPECT_EQ(cs.size(), 0);
  EXPECT_TRUE(cs.data() == nullptr);
}

TEST(span_test, size_optimization) {
  hcoona::span<int> s;
  EXPECT_TRUE(sizeof(s) == sizeof(int*) + sizeof(ptrdiff_t));

  hcoona::span<int, 0> se;
  EXPECT_TRUE(sizeof(se) == sizeof(int*));
}

TEST(span_test, from_nullptr_size_constructor) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. from_nullptr_size_constructor";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  {
    hcoona::span<int> s{nullptr,
                        hcoona::narrow_cast<hcoona::span<int>::size_type>(0)};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == nullptr);

    hcoona::span<int> cs{nullptr,
                         hcoona::narrow_cast<hcoona::span<int>::size_type>(0)};
    EXPECT_EQ(cs.size(), 0);
    EXPECT_TRUE(cs.data() == nullptr);
  }
  {
    auto workaround_macro = []() {
      const hcoona::span<int, 1> s{
          nullptr, hcoona::narrow_cast<hcoona::span<int>::size_type>(0)};
    };
    EXPECT_DEATH(workaround_macro(), expected);
  }
  {
    auto workaround_macro = []() { const hcoona::span<int> s{nullptr, 1}; };
    EXPECT_DEATH(workaround_macro(), expected);

    auto const_workaround_macro = []() {
      const hcoona::span<const int> s{nullptr, 1};
    };
    EXPECT_DEATH(const_workaround_macro(), expected);
  }
  {
    auto workaround_macro = []() { const hcoona::span<int, 0> s{nullptr, 1}; };
    EXPECT_DEATH(workaround_macro(), expected);

    auto const_workaround_macro = []() {
      const hcoona::span<const int, 0> s{nullptr, 1};
    };
    EXPECT_DEATH(const_workaround_macro(), expected);
  }
  {
    hcoona::span<int*> s{nullptr,
                         hcoona::narrow_cast<hcoona::span<int>::size_type>(0)};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == nullptr);

    hcoona::span<const int*> cs{
        nullptr, hcoona::narrow_cast<hcoona::span<int>::size_type>(0)};
    EXPECT_EQ(cs.size(), 0);
    EXPECT_TRUE(cs.data() == nullptr);
  }
}

TEST(span_test, from_pointer_length_constructor) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. from_pointer_length_constructor";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int arr[4] = {1, 2, 3, 4};

  {
    for (int i = 0; i < 4; ++i) {
      {
        hcoona::span<int> s = {&arr[0], hcoona::narrow_cast<std::size_t>(i)};
        EXPECT_TRUE(s.size() == hcoona::narrow_cast<std::size_t>(i));
        EXPECT_TRUE(s.data() == &arr[0]);
        EXPECT_TRUE(s.empty() == (i == 0));
        for (int j = 0; j < i; ++j) {
          EXPECT_EQ(arr[j], s[hcoona::narrow_cast<std::size_t>(j)]);
        }
      }
      {
        hcoona::span<int> s = {&arr[i],
                               4 - hcoona::narrow_cast<std::size_t>(i)};
        EXPECT_TRUE(s.size() == 4 - hcoona::narrow_cast<std::size_t>(i));
        EXPECT_TRUE(s.data() == &arr[i]);
        EXPECT_TRUE(s.empty() == ((4 - i) == 0));

        for (int j = 0; j < 4 - i; ++j) {
          EXPECT_EQ(arr[j + i], s[hcoona::narrow_cast<std::size_t>(j)]);
        }
      }
    }
  }

  {
    hcoona::span<int, 2> s{&arr[0], 2};
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == &arr[0]);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
  }

  {
    int* p = nullptr;
    hcoona::span<int> s{p,
                        hcoona::narrow_cast<hcoona::span<int>::size_type>(0)};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == nullptr);
  }

  {
    int* p = nullptr;
    auto workaround_macro = [=]() { const hcoona::span<int> s{p, 2}; };
    EXPECT_DEATH(workaround_macro(), expected);
  }
}

TEST(span_test, from_pointer_pointer_construction) {
  int arr[4] = {1, 2, 3, 4};

  {
    hcoona::span<int> s{&arr[0], &arr[2]};
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == &arr[0]);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
  }
  {
    hcoona::span<int, 2> s{&arr[0], &arr[2]};
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == &arr[0]);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[1], 2);
  }

  {
    hcoona::span<int> s{&arr[0], &arr[0]};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == &arr[0]);
  }

  {
    hcoona::span<int, 0> s{&arr[0], &arr[0]};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == &arr[0]);
  }

  // this will fail the std::distance() precondition, which asserts on MSVC
  // debug builds
  //{
  //    auto workaround_macro = [&]() { hcoona::span<int> s{&arr[1], &arr[0]};
  //    }; EXPECT_DEATH(workaround_macro(), expected);
  //}

  // this will fail the std::distance() precondition, which asserts on MSVC
  // debug builds
  //{
  //    int* p = nullptr;
  //    auto workaround_macro = [&]() { hcoona::span<int> s{&arr[0], p}; };
  //    EXPECT_DEATH(workaround_macro(), expected);
  //}

  {
    int* p = nullptr;
    hcoona::span<int> s{p, p};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == nullptr);
  }

  {
    int* p = nullptr;
    hcoona::span<int, 0> s{p, p};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == nullptr);
  }

  // this will fail the std::distance() precondition, which asserts on MSVC
  // debug builds
  //{
  //    int* p = nullptr;
  //    auto workaround_macro = [&]() { hcoona::span<int> s{&arr[0], p}; };
  //    EXPECT_DEATH(workaround_macro(), expected);
  //}
}

TEST(span_test, from_array_constructor) {
  int arr[5] = {1, 2, 3, 4, 5};

  {
    const hcoona::span<int> s{arr};
    EXPECT_EQ(s.size(), 5);
    EXPECT_TRUE(s.data() == &arr[0]);
  }

  {
    const hcoona::span<int, 5> s{arr};
    EXPECT_EQ(s.size(), 5);
    EXPECT_TRUE(s.data() == &arr[0]);
  }

  int arr2d[2][3] = {{1, 2, 3}, {4, 5, 6}};

#ifdef CONFIRM_COMPILATION_ERRORS
  { hcoona::span<int, 6> s{arr}; }

  {
    hcoona::span<int, 0> s{arr};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == &arr[0]);
  }

  {
    hcoona::span<int> s{arr2d};
    EXPECT_EQ(s.size(), 6);
    EXPECT_TRUE(s.data() == &arr2d[0][0]);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[5], 6);
  }

  {
    hcoona::span<int, 0> s{arr2d};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == &arr2d[0][0]);
  }

  { hcoona::span<int, 6> s{arr2d}; }
#endif
  {
    const hcoona::span<int[3]> s{std::addressof(arr2d[0]), 1};
    EXPECT_EQ(s.size(), 1);
    EXPECT_TRUE(s.data() == std::addressof(arr2d[0]));
  }

  int arr3d[2][3][2] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

#ifdef CONFIRM_COMPILATION_ERRORS
  {
    hcoona::span<int> s{arr3d};
    EXPECT_EQ(s.size(), 12);
    EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[11], 12);
  }

  {
    hcoona::span<int, 0> s{arr3d};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
  }

  { hcoona::span<int, 11> s{arr3d}; }

  {
    hcoona::span<int, 12> s{arr3d};
    EXPECT_EQ(s.size(), 12);
    EXPECT_TRUE(s.data() == &arr3d[0][0][0]);
    EXPECT_EQ(s[0], 1);
    EXPECT_EQ(s[5], 6);
  }
#endif
  {
    const hcoona::span<int[3][2]> s{std::addressof(arr3d[0]), 1};
    EXPECT_EQ(s.size(), 1);
  }

  AddressOverloaded ao_arr[5] = {};

  {
    const hcoona::span<AddressOverloaded, 5> s{ao_arr};
    EXPECT_EQ(s.size(), 5);
    EXPECT_TRUE(s.data() == std::addressof(ao_arr[0]));
  }
}

TEST(span_test, from_dynamic_array_constructor) {
  double(*arr)[3][4] = new double[100][3][4];

  {
    hcoona::span<double> s(&arr[0][0][0], 10);
    EXPECT_EQ(s.size(), 10);
    EXPECT_TRUE(s.data() == &arr[0][0][0]);
  }

  delete[] arr;
}

TEST(span_test, from_std_array_constructor) {
  std::array<int, 4> arr = {1, 2, 3, 4};

  {
    hcoona::span<int> s{arr};
    EXPECT_TRUE(s.size() == arr.size());
    EXPECT_TRUE(s.data() == arr.data());

    hcoona::span<const int> cs{arr};
    EXPECT_TRUE(cs.size() == arr.size());
    EXPECT_TRUE(cs.data() == arr.data());
  }

  {
    hcoona::span<int, 4> s{arr};
    EXPECT_TRUE(s.size() == arr.size());
    EXPECT_TRUE(s.data() == arr.data());

    hcoona::span<const int, 4> cs{arr};
    EXPECT_TRUE(cs.size() == arr.size());
    EXPECT_TRUE(cs.data() == arr.data());
  }

  {
    std::array<int, 0> empty_arr{};
    hcoona::span<int> s{empty_arr};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.empty());
  }

  std::array<AddressOverloaded, 4> ao_arr{};

  {
    hcoona::span<AddressOverloaded, 4> fs{ao_arr};
    EXPECT_TRUE(fs.size() == ao_arr.size());
    EXPECT_TRUE(ao_arr.data() == fs.data());
  }

#ifdef CONFIRM_COMPILATION_ERRORS
  {
    hcoona::span<int, 2> s{arr};
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == arr.data());

    hcoona::span<const int, 2> cs{arr};
    EXPECT_EQ(cs.size(), 2);
    EXPECT_TRUE(cs.data() == arr.data());
  }

  {
    hcoona::span<int, 0> s{arr};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == arr.data());

    hcoona::span<const int, 0> cs{arr};
    EXPECT_EQ(cs.size(), 0);
    EXPECT_TRUE(cs.data() == arr.data());
  }

  { hcoona::span<int, 5> s{arr}; }

  {
    auto get_an_array = []() -> std::array<int, 4> { return {1, 2, 3, 4}; };
    auto take_a_span = [](hcoona::span<int> s) { static_cast<void>(s); };
    // try to take a temporary std::array
    take_a_span(get_an_array());
  }
#endif

  {
    auto get_an_array = []() -> std::array<int, 4> { return {1, 2, 3, 4}; };
    auto take_a_span = [](hcoona::span<const int> s) { static_cast<void>(s); };
    // try to take a temporary std::array
    take_a_span(get_an_array());
  }
}

TEST(span_test, from_const_std_array_constructor) {
  const std::array<int, 4> arr = {1, 2, 3, 4};

  {
    hcoona::span<const int> s{arr};
    EXPECT_TRUE(s.size() == arr.size());
    EXPECT_TRUE(s.data() == arr.data());
  }

  {
    hcoona::span<const int, 4> s{arr};
    EXPECT_TRUE(s.size() == arr.size());
    EXPECT_TRUE(s.data() == arr.data());
  }

  const std::array<AddressOverloaded, 4> ao_arr{};

  {
    hcoona::span<const AddressOverloaded, 4> s{ao_arr};
    EXPECT_TRUE(s.size() == ao_arr.size());
    EXPECT_TRUE(s.data() == ao_arr.data());
  }

#ifdef CONFIRM_COMPILATION_ERRORS
  {
    hcoona::span<const int, 2> s{arr};
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == arr.data());
  }

  {
    hcoona::span<const int, 0> s{arr};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == arr.data());
  }

  { hcoona::span<const int, 5> s{arr}; }
#endif

  {
    auto get_an_array = []() -> const std::array<int, 4> {
      return {1, 2, 3, 4};
    };
    auto take_a_span = [](hcoona::span<const int> s) { static_cast<void>(s); };
    // try to take a temporary std::array
    take_a_span(get_an_array());
  }
}

TEST(span_test, from_std_array_const_constructor) {
  std::array<const int, 4> arr = {1, 2, 3, 4};

  {
    hcoona::span<const int> s{arr};
    EXPECT_TRUE(s.size() == arr.size());
    EXPECT_TRUE(s.data() == arr.data());
  }

  {
    hcoona::span<const int, 4> s{arr};
    EXPECT_TRUE(s.size() == arr.size());
    EXPECT_TRUE(s.data() == arr.data());
  }

#ifdef CONFIRM_COMPILATION_ERRORS
  {
    hcoona::span<const int, 2> s{arr};
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == arr.data());
  }

  {
    hcoona::span<const int, 0> s{arr};
    EXPECT_EQ(s.size(), 0);
    EXPECT_TRUE(s.data() == arr.data());
  }

  { hcoona::span<const int, 5> s{arr}; }

  { hcoona::span<int, 4> s{arr}; }
#endif
}

TEST(span_test, from_container_constructor) {
  std::vector<int> v = {1, 2, 3};
  const std::vector<int> cv = v;

  {
    hcoona::span<int> s{v};
    EXPECT_TRUE(s.size() == v.size());
    EXPECT_TRUE(s.data() == v.data());

    hcoona::span<const int> cs{v};
    EXPECT_TRUE(cs.size() == v.size());
    EXPECT_TRUE(cs.data() == v.data());
  }

  std::string str = "hello";
  const std::string cstr = "hello";

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    hcoona::span<char> s{str};
    EXPECT_TRUE(s.size() == str.size());
         EXPECT_TRUE(s.data() == str.data()));
#endif
         hcoona::span<const char> cs{str};
         EXPECT_TRUE(cs.size() == str.size());
         EXPECT_TRUE(cs.data() == str.data());
  }

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    hcoona::span<char> s{cstr};
#endif
    hcoona::span<const char> cs{cstr};
    EXPECT_TRUE(cs.size() == cstr.size());
    EXPECT_TRUE(cs.data() == cstr.data());
  }

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    auto get_temp_vector = []() -> std::vector<int> { return {}; };
    auto use_span = [](hcoona::span<int> s) { static_cast<void>(s); };
    use_span(get_temp_vector());
#endif
  }

  {
    auto get_temp_vector = []() -> std::vector<int> { return {}; };
    auto use_span = [](hcoona::span<const int> s) { static_cast<void>(s); };
    use_span(get_temp_vector());
  }

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    auto get_temp_string = []() -> std::string { return {}; };
    auto use_span = [](hcoona::span<char> s) { static_cast<void>(s); };
    use_span(get_temp_string());
#endif
  }

  {
    auto get_temp_string = []() -> std::string { return {}; };
    auto use_span = [](hcoona::span<const char> s) { static_cast<void>(s); };
    use_span(get_temp_string());
  }

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    auto get_temp_vector = []() -> const std::vector<int> { return {}; };
    auto use_span = [](hcoona::span<const char> s) { static_cast<void>(s); };
    use_span(get_temp_vector());
#endif
  }

  {
    auto get_temp_string = []() -> const std::string { return {}; };
    auto use_span = [](hcoona::span<const char> s) { static_cast<void>(s); };
    use_span(get_temp_string());
  }

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    std::map<int, int> m;
    hcoona::span<int> s{m};
#endif
  }
}

TEST(span_test,
     from_convertible_span_constructor){{hcoona::span<DerivedClass> avd;
hcoona::span<const DerivedClass> avcd = avd;
static_cast<void>(avcd);
}

{
#ifdef CONFIRM_COMPILATION_ERRORS
  hcoona::span<DerivedClass> avd;
  hcoona::span<BaseClass> avb = avd;
  static_cast<void>(avb);
#endif
}

#ifdef CONFIRM_COMPILATION_ERRORS
{
  hcoona::span<int> s;
  hcoona::span<unsigned int> s2 = s;
  static_cast<void>(s2);
}

{
  hcoona::span<int> s;
  hcoona::span<const unsigned int> s2 = s;
  static_cast<void>(s2);
}

{
  hcoona::span<int> s;
  hcoona::span<short> s2 = s;
  static_cast<void>(s2);
}
#endif
}

TEST(span_test, copy_move_and_assignment) {
  hcoona::span<int> s1;
  EXPECT_TRUE(s1.empty());

  int arr[] = {3, 4, 5};

  hcoona::span<const int> s2 = arr;
  EXPECT_EQ(s2.size(), 3);
  EXPECT_TRUE(s2.data() == &arr[0]);

  s2 = s1;
  EXPECT_TRUE(s2.empty());

  auto get_temp_span = [&]() -> hcoona::span<int> { return {&arr[1], 2}; };
  auto use_span = [&](hcoona::span<const int> s) {
    EXPECT_EQ(s.size(), 2);
    EXPECT_TRUE(s.data() == &arr[1]);
  };
  use_span(get_temp_span());

  s1 = get_temp_span();
  EXPECT_EQ(s1.size(), 2);
  EXPECT_TRUE(s1.data() == &arr[1]);
}

TEST(span_test, first) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. first";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int arr[5] = {1, 2, 3, 4, 5};

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.first<2>().size(), 2);
    EXPECT_EQ(av.first(2).size(), 2);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.first<0>().size(), 0);
    EXPECT_EQ(av.first(0).size(), 0);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.first<5>().size(), 5);
    EXPECT_EQ(av.first(5).size(), 5);
  }

  {
    hcoona::span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
    EXPECT_EQ(av.first<6>().size(), 6);
    EXPECT_TRUE(av.first<-1>().size() == -1);
#endif
    EXPECT_DEATH(av.first(6).size(), expected);
  }

  {
    hcoona::span<int> av;
    EXPECT_EQ(av.first<0>().size(), 0);
    EXPECT_EQ(av.first(0).size(), 0);
  }
}

TEST(span_test, last) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. last";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int arr[5] = {1, 2, 3, 4, 5};

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.last<2>().size(), 2);
    EXPECT_EQ(av.last(2).size(), 2);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.last<0>().size(), 0);
    EXPECT_EQ(av.last(0).size(), 0);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.last<5>().size(), 5);
    EXPECT_EQ(av.last(5).size(), 5);
  }

  {
    hcoona::span<int, 5> av = arr;
#ifdef CONFIRM_COMPILATION_ERRORS
    EXPECT_EQ(av.last<6>().size(), 6);
#endif
    EXPECT_DEATH(av.last(6).size(), expected);
  }

  {
    hcoona::span<int> av;
    EXPECT_EQ(av.last<0>().size(), 0);
    EXPECT_EQ(av.last(0).size(), 0);
  }
}

TEST(span_test, subspan) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. subspan";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int arr[5] = {1, 2, 3, 4, 5};

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ((av.subspan<2, 2>().size()), 2);
    EXPECT_EQ(decltype(av.subspan<2, 2>())::extent, 2);
    EXPECT_EQ(av.subspan(2, 2).size(), 2);
    EXPECT_EQ(av.subspan(2, 3).size(), 3);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ((av.subspan<0, 0>().size()), 0);
    EXPECT_EQ(decltype(av.subspan<0, 0>())::extent, 0);
    EXPECT_EQ(av.subspan(0, 0).size(), 0);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ((av.subspan<0, 5>().size()), 5);
    EXPECT_EQ(decltype(av.subspan<0, 5>())::extent, 5);
    EXPECT_EQ(av.subspan(0, 5).size(), 5);

    EXPECT_DEATH(av.subspan(0, 6).size(), expected);
    EXPECT_DEATH(av.subspan(1, 5).size(), expected);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ((av.subspan<4, 0>().size()), 0);
    EXPECT_EQ(decltype(av.subspan<4, 0>())::extent, 0);
    EXPECT_EQ(av.subspan(4, 0).size(), 0);
    EXPECT_EQ(av.subspan(5, 0).size(), 0);
    EXPECT_DEATH(av.subspan(6, 0).size(), expected);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.subspan<1>().size(), 4);
    EXPECT_EQ(decltype(av.subspan<1>())::extent, 4);
  }

  {
    hcoona::span<int> av;
    EXPECT_EQ((av.subspan<0, 0>().size()), 0);
    EXPECT_EQ(decltype(av.subspan<0, 0>())::extent, 0);
    EXPECT_EQ(av.subspan(0, 0).size(), 0);
    EXPECT_DEATH((av.subspan<1, 0>().size()), expected);
  }

  {
    hcoona::span<int> av;
    EXPECT_EQ(av.subspan(0).size(), 0);
    EXPECT_DEATH(av.subspan(1).size(), expected);
  }

  {
    hcoona::span<int> av = arr;
    EXPECT_EQ(av.subspan(0).size(), 5);
    EXPECT_EQ(av.subspan(1).size(), 4);
    EXPECT_EQ(av.subspan(4).size(), 1);
    EXPECT_EQ(av.subspan(5).size(), 0);
    EXPECT_DEATH(av.subspan(6).size(), expected);
    const auto av2 = av.subspan(1);
    for (std::size_t i = 0; i < 4; ++i)
      EXPECT_TRUE(av2[i] == static_cast<int>(i) + 2);
  }

  {
    hcoona::span<int, 5> av = arr;
    EXPECT_EQ(av.subspan(0).size(), 5);
    EXPECT_EQ(av.subspan(1).size(), 4);
    EXPECT_EQ(av.subspan(4).size(), 1);
    EXPECT_EQ(av.subspan(5).size(), 0);
    EXPECT_DEATH(av.subspan(6).size(), expected);
    const auto av2 = av.subspan(1);
    for (std::size_t i = 0; i < 4; ++i)
      EXPECT_TRUE(av2[i] == static_cast<int>(i) + 2);
  }
}

TEST(span_test, iterator_default_init) {
  hcoona::span<int>::iterator it1;
  hcoona::span<int>::iterator it2;
  EXPECT_TRUE(it1 == it2);
}

TEST(span_test, iterator_comparisons) {
  int a[] = {1, 2, 3, 4};
  {
    hcoona::span<int> s = a;
    hcoona::span<int>::iterator it = s.begin();
    auto it2 = it + 1;

    EXPECT_TRUE(it == it);
    EXPECT_TRUE(it == s.begin());
    EXPECT_TRUE(s.begin() == it);

    EXPECT_TRUE(it != it2);
    EXPECT_TRUE(it2 != it);
    EXPECT_TRUE(it != s.end());
    EXPECT_TRUE(it2 != s.end());
    EXPECT_TRUE(s.end() != it);

    EXPECT_TRUE(it < it2);
    EXPECT_TRUE(it <= it2);
    EXPECT_TRUE(it2 <= s.end());
    EXPECT_TRUE(it < s.end());

    EXPECT_TRUE(it2 > it);
    EXPECT_TRUE(it2 >= it);
    EXPECT_TRUE(s.end() > it2);
    EXPECT_TRUE(s.end() >= it2);
  }
}

TEST(span_test, incomparable_iterators) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. incomparable_iterators";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int a[] = {1, 2, 3, 4};
  int b[] = {1, 2, 3, 4};
  {
    hcoona::span<int> s = a;
    hcoona::span<int> s2 = b;
#if (__cplusplus > 201402L)
    EXPECT_DEATH([[maybe_unused]] bool _ = (s.begin() == s2.begin()), expected);
    EXPECT_DEATH([[maybe_unused]] bool _ = (s.begin() <= s2.begin()), expected);
#else
    EXPECT_DEATH(bool _ = (s.begin() == s2.begin()), expected);
    EXPECT_DEATH(bool _ = (s.begin() <= s2.begin()), expected);
#endif
  }
}

TEST(span_test, begin_end) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. begin_end";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  {
    int a[] = {1, 2, 3, 4};
    hcoona::span<int> s = a;

    hcoona::span<int>::iterator it = s.begin();
    hcoona::span<int>::iterator it2 = std::begin(s);
    EXPECT_TRUE(it == it2);

    it = s.end();
    it2 = std::end(s);
    EXPECT_TRUE(it == it2);
  }

  {
    int a[] = {1, 2, 3, 4};
    hcoona::span<int> s = a;

    auto it = s.begin();
    auto first = it;
    EXPECT_TRUE(it == first);
    EXPECT_EQ(*it, 1);

    auto beyond = s.end();
    EXPECT_TRUE(it != beyond);
    EXPECT_DEATH(*beyond, expected);

    EXPECT_EQ(beyond - first, 4);
    EXPECT_EQ(first - first, 0);
    EXPECT_EQ(beyond - beyond, 0);

    ++it;
    EXPECT_EQ(it - first, 1);
    EXPECT_EQ(*it, 2);
    *it = 22;
    EXPECT_EQ(*it, 22);
    EXPECT_EQ(beyond - it, 3);

    it = first;
    EXPECT_TRUE(it == first);
    while (it != s.end()) {
      *it = 5;
      ++it;
    }

    EXPECT_TRUE(it == beyond);
    EXPECT_EQ(it - beyond, 0);

    for (const auto& n : s) {
      EXPECT_EQ(n, 5);
    }
  }
}

TEST(span_test, rbegin_rend) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. rbegin_rend";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  {
    int a[] = {1, 2, 3, 4};
    hcoona::span<int> s = a;

    auto it = s.rbegin();
    auto first = it;
    EXPECT_TRUE(it == first);
    EXPECT_EQ(*it, 4);

    auto beyond = s.rend();
    EXPECT_TRUE(it != beyond);
#if (__cplusplus > 201402L)
    EXPECT_DEATH([[maybe_unused]] auto _ = *beyond, expected);
#else
    EXPECT_DEATH(auto _ = *beyond, expected);
#endif

    EXPECT_EQ(beyond - first, 4);
    EXPECT_EQ(first - first, 0);
    EXPECT_EQ(beyond - beyond, 0);

    ++it;
    EXPECT_EQ(it - s.rbegin(), 1);
    EXPECT_EQ(*it, 3);
    *it = 22;
    EXPECT_EQ(*it, 22);
    EXPECT_EQ(beyond - it, 3);

    it = first;
    EXPECT_TRUE(it == first);
    while (it != s.rend()) {
      *it = 5;
      ++it;
    }

    EXPECT_TRUE(it == beyond);
    EXPECT_EQ(it - beyond, 0);

    for (const auto& n : s) {
      EXPECT_EQ(n, 5);
    }
  }
}

TEST(span_test, as_bytes) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. as_bytes";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int a[] = {1, 2, 3, 4};
  {
    const hcoona::span<const int> s = a;
    EXPECT_EQ(s.size(), 4);
    const hcoona::span<const std::byte> bs = as_bytes(s);
    EXPECT_TRUE(static_cast<const void*>(bs.data()) ==
                static_cast<const void*>(s.data()));
    EXPECT_TRUE(bs.size() == s.size_bytes());
  }

  {
    hcoona::span<int> s;
    const auto bs = as_bytes(s);
    EXPECT_TRUE(bs.size() == s.size());
    EXPECT_EQ(bs.size(), 0);
    EXPECT_EQ(bs.size_bytes(), 0);
    EXPECT_TRUE(static_cast<const void*>(bs.data()) ==
                static_cast<const void*>(s.data()));
    EXPECT_TRUE(bs.data() == nullptr);
  }

  {
    hcoona::span<int> s = a;
    const auto bs = as_bytes(s);
    EXPECT_TRUE(static_cast<const void*>(bs.data()) ==
                static_cast<const void*>(s.data()));
    EXPECT_TRUE(bs.size() == s.size_bytes());
  }

  int b[5] = {1, 2, 3, 4, 5};
  {
    hcoona::span<int> sp(std::begin(b), static_cast<size_t>(-2));
    EXPECT_DEATH((void)sp.size_bytes(), expected);
  }
}

TEST(span_test, as_writable_bytes) {
  int a[] = {1, 2, 3, 4};

  {
#ifdef CONFIRM_COMPILATION_ERRORS
    // you should not be able to get writeable bytes for const objects
    hcoona::span<const int> s = a;
    EXPECT_EQ(s.size(), 4);
    hcoona::span<const byte> bs = as_writable_bytes(s);
    EXPECT_TRUE(static_cast<void*>(bs.data()) == static_cast<void*>(s.data()));
    EXPECT_TRUE(bs.size() == s.size_bytes());
#endif
  }

  {
    hcoona::span<int> s;
    const auto bs = as_writable_bytes(s);
    EXPECT_TRUE(bs.size() == s.size());
    EXPECT_EQ(bs.size(), 0);
    EXPECT_EQ(bs.size_bytes(), 0);
    EXPECT_TRUE(static_cast<void*>(bs.data()) == static_cast<void*>(s.data()));
    EXPECT_TRUE(bs.data() == nullptr);
  }

  {
    hcoona::span<int> s = a;
    const auto bs = as_writable_bytes(s);
    EXPECT_TRUE(static_cast<void*>(bs.data()) == static_cast<void*>(s.data()));
    EXPECT_TRUE(bs.size() == s.size_bytes());
  }
}

TEST(span_test, fixed_size_conversions) {
  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. fixed_size_conversions";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  int arr[] = {1, 2, 3, 4};

  // converting to an span from an equal size array is ok
  hcoona::span<int, 4> s4 = arr;
  EXPECT_EQ(s4.size(), 4);

  // converting to dynamic_range is always ok
  {
    hcoona::span<int> s = s4;
    EXPECT_EQ(s.size(), s4.size());
    static_cast<void>(s);
  }

// initialization or assignment to static span that REDUCES size is NOT ok
#ifdef CONFIRM_COMPILATION_ERRORS
  { hcoona::span<int, 2> s = arr; }
  {
    hcoona::span<int, 2> s2 = s4;
    static_cast<void>(s2);
  }
#endif

  // even when done dynamically
  {
      /*
      // this now results in a compile-time error, rather than runtime.
      // There is no suitable conversion from dynamic span to fixed span.
      hcoona::span<int> s = arr;
      auto f = [&]() {
          const hcoona::span<int, 2> s2 = s;
          static_cast<void>(s2);
      };
      EXPECT_DEATH(f(), expected);
      */
  }

  // but doing so explicitly is ok

  // you can convert statically
  {
    const hcoona::span<int, 2> s2{&arr[0], 2};
    static_cast<void>(s2);
  }
  {
    const hcoona::span<int, 1> s1 = s4.first<1>();
    static_cast<void>(s1);
  }

  /*
   // this is not a legal operation in std::span, so we are no longer supporting
   it
   // conversion from hcoona::span<int, 4> to hcoona::span<int, dynamic_extent>
   via call to `first`
   // then convert from hcoona::span<int, dynamic_extent> to hcoona::span<int,
   1>
   // The dynamic to fixed extents are not supported in the standard
   // to make this work, hcoona::span<int, 1> would need to be
   hcoona::span<int>.
   {
       // NB: implicit conversion to hcoona::span<int,1> from hcoona::span<int>
       hcoona::span<int, 1> s1 = s4.first(1);
       static_cast<void>(s1);
   }
   */

  // initialization or assignment to static span that requires size INCREASE is
  // not ok.
  int arr2[2] = {1, 2};

#ifdef CONFIRM_COMPILATION_ERRORS
  { hcoona::span<int, 4> s3 = arr2; }
  {
    hcoona::span<int, 2> s2 = arr2;
    hcoona::span<int, 4> s4a = s2;
  }
#endif
  {
    auto f = [&]() {
      const hcoona::span<int, 4> _s4{arr2, 2};
      static_cast<void>(_s4);
    };
    EXPECT_DEATH(f(), expected);
  }

  /*
   // This no longer compiles. There is no suitable conversion from dynamic span
   to a fixed size span.
   // this should fail - we are trying to assign a small dynamic span to a
   fixed_size larger one hcoona::span<int> av = arr2; auto f = [&]() { const
   hcoona::span<int, 4> _s4 = av; static_cast<void>(_s4);
   };
   EXPECT_DEATH(f(), expected);
   */
}

TEST(span_test, interop_with_std_regex) {
  char lat[] = {'1', '2', '3', '4', '5', '6', 'E', 'F', 'G'};
  hcoona::span<char> s = lat;
  const auto f_it = s.begin() + 7;

  std::match_results<hcoona::span<char>::iterator> match;

  std::regex_match(s.begin(), s.end(), match, std::regex(".*"));
  EXPECT_TRUE(match.ready());
  EXPECT_FALSE(match.empty());
  EXPECT_TRUE(match[0].matched);
  EXPECT_TRUE(match[0].first == s.begin());
  EXPECT_TRUE(match[0].second == s.end());

  std::regex_search(s.begin(), s.end(), match, std::regex("F"));
  EXPECT_TRUE(match.ready());
  EXPECT_FALSE(match.empty());
  EXPECT_TRUE(match[0].matched);
  EXPECT_TRUE(match[0].first == f_it);
  EXPECT_TRUE(match[0].second == (f_it + 1));
}

TEST(span_test, default_constructible) {
  EXPECT_TRUE((std::is_default_constructible<hcoona::span<int>>::value));
  EXPECT_TRUE((std::is_default_constructible<hcoona::span<int, 0>>::value));
  EXPECT_FALSE((std::is_default_constructible<hcoona::span<int, 42>>::value));
}

TEST(span_test, front_back) {
  int arr[5] = {1, 2, 3, 4, 5};
  hcoona::span<int> s{arr};
  EXPECT_EQ(s.front(), 1);
  EXPECT_EQ(s.back(), 5);

  const auto terminateHandler = std::set_terminate([] {
    std::cerr << "Expected Death. front_back";
    std::abort();
  });
  const auto* const expected = GetExpectedDeathString(terminateHandler);

  hcoona::span<int> s2;
  EXPECT_DEATH(s2.front(), expected);
  EXPECT_DEATH(s2.back(), expected);
}
