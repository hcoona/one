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
// This file header defines utility pre-processor macros.
//
// ONE_PREDICT_TRUE, ONE_PREDICT_FALSE, ONE_ASSERT, ONE_INTERNAL_HARDENING_ABORT
// and ONE_HARDENING_ASSERT are coming from
// https://github.com/abseil/abseil-cpp/blob/c5a424a2a21005660b182516eb7a079cd8021699/absl/base/optimization.h
//
// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <utility>

// ONE_PREDICT_TRUE, ONE_PREDICT_FALSE
//
// Enables the compiler to prioritize compilation using static analysis for
// likely paths within a boolean branch.
//
// Example:
//
//   if (ONE_PREDICT_TRUE(expression)) {
//     return result;                        // Faster if more likely
//   } else {
//     return 0;
//   }
//
// Compilers can use the information that a certain branch is not likely to be
// taken (for instance, a CHECK failure) to optimize for the common case in
// the absence of better information (ie. compiling gcc with `-fprofile-arcs`).
//
// Recommendation: Modern CPUs dynamically predict branch execution paths,
// typically with accuracy greater than 97%. As a result, annotating every
// branch in a codebase is likely counterproductive; however, annotating
// specific branches that are both hot and consistently mispredicted is likely
// to yield performance improvements.
#define ONE_PREDICT_FALSE(x) (__builtin_expect(false || (x), false))
#define ONE_PREDICT_TRUE(x) (__builtin_expect(false || (x), true))

#define ONE_RETURN_IF_NOT_OK(status)    \
  do {                                  \
    auto&& __s = (status);              \
    if (ONE_PREDICT_FALSE(!__s.ok())) { \
      return __s;                       \
    }                                   \
  } while (false)

#define ONE_STRINGIFY(x) #x
#define ONE_CONCAT(x, y) x##y

#define ONE_ASSIGN_OR_RETURN_IMPL(result_name, lhs, rexpr) \
  auto&& result_name = (rexpr);                            \
  do {                                                     \
    if (ONE_PREDICT_FALSE(!result_name.ok())) {            \
      return result_name.status();                         \
    }                                                      \
  } while (false);                                         \
  lhs = std::move(result_name).value();

#define ONE_ASSIGN_OR_RETURN_NAME(x, y) ONE_CONCAT(x, y)

#define ONE_ASSIGN_OR_RETURN(lhs, rexpr)                                       \
  ONE_ASSIGN_OR_RETURN_IMPL(ONE_ASSIGN_OR_RETURN_NAME(status_or, __COUNTER__), \
                            lhs, rexpr);

// ONE_ASSERT()
//
// In C++11, `assert` can't be used portably within constexpr functions.
// ONE_ASSERT functions as a runtime assert but works in C++11 constexpr
// functions.  Example:
//
// constexpr double Divide(double a, double b) {
//   return ONE_ASSERT(b != 0), a / b;
// }
//
// This macro is inspired by
// https://akrzemi1.wordpress.com/2017/05/18/asserts-in-constexpr-functions/
#if defined(NDEBUG)
#define ONE_ASSERT(expr) \
  (false ? static_cast<void>(expr) : static_cast<void>(0))
#else
#define ONE_ASSERT(expr)                           \
  (ONE_PREDICT_TRUE((expr)) ? static_cast<void>(0) \
                            : [] { assert(false && #expr); }())  // NOLINT
#endif

// `ONE_INTERNAL_HARDENING_ABORT()` controls how `ONE_HARDENING_ASSERT()`
// aborts the program in release mode (when NDEBUG is defined). The
// implementation should abort the program as quickly as possible and ideally it
// should not be possible to ignore the abort request.
#if (__has_builtin(__builtin_trap) && __has_builtin(__builtin_unreachable)) || \
    (defined(__GNUC__) && !defined(__clang__))
#define ONE_INTERNAL_HARDENING_ABORT() \
  do {                                 \
    __builtin_trap();                  \
    __builtin_unreachable();           \
  } while (false)
#else
#define ONE_INTERNAL_HARDENING_ABORT() abort()
#endif

// ONE_HARDENING_ASSERT()
//
// `ONE_HARDENING_ASSERT()` is like `ONE_ASSERT()`, but used to implement
// runtime assertions that should be enabled in hardened builds even when
// `NDEBUG` is defined.
//
// When `NDEBUG` is not defined, `ONE_HARDENING_ASSERT()` is identical to
// `ONE_ASSERT()`.
//
// See `ONE_OPTION_HARDENED` in `absl/base/options.h` for more information on
// hardened mode.
#if ONE_OPTION_HARDENED == 1 && defined(NDEBUG)
#define ONE_HARDENING_ASSERT(expr)                 \
  (ONE_PREDICT_TRUE((expr)) ? static_cast<void>(0) \
                            : [] { ONE_INTERNAL_HARDENING_ABORT(); }())
#else
#define ONE_HARDENING_ASSERT(expr) ONE_ASSERT(expr)
#endif
