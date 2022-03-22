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
// Copied from https://github.com/microsoft/GSL/blob/v4.0.0/include/gsl/narrow
// for `gsl::narrow()` and
// https://github.com/microsoft/GSL/blob/v4.0.0/include/gsl/util for
// `gsl::narrow_cast()`.
//
// Modifications:
// 1. Use #pragma once header guard.
// 2. Re-format with Google style.
// 3. Replace gsl namespace with hcoona.
// 3. Remove GSL_SUPPRESS.
// 4. Use hardening check instead of throw an exception.
// 5. Move the origin `gsl::narrow_cast()` to `details::NarrowCastImpl()` & move
//    the origin `gsl::narrow()` to `narrow_cast()`.
//
// Copyright (c) 2015 Microsoft Corporation. All rights reserved.
//
// This code is licensed under the MIT License (MIT).
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include <utility>

#include "one/base/macros.h"

namespace hcoona {

namespace details {

template <class T, class U>
constexpr T NarrowCastImpl(U&& u) noexcept {
  return static_cast<T>(std::forward<U>(u));
}

}  // namespace details

template <
    class T, class U,
    typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
constexpr T narrow_cast(U u) noexcept {
  constexpr const bool is_different_signedness =
      (std::is_signed<T>::value != std::is_signed<U>::value);

  // While this is technically undefined behavior in some cases (i.e., if the
  // source value is of floating-point type and cannot fit into the destination
  // integral type), the resultant behavior is benign on the platforms that we
  // target (i.e., no hardware trap representations are hit).
  const T t = details::NarrowCastImpl<T>(u);

  ONE_HARDENING_ASSERT(!(static_cast<U>(t) != u || (is_different_signedness &&
                                                    ((t < T{}) != (u < U{})))));

  return t;
}

template <
    class T, class U,
    typename std::enable_if<!std::is_arithmetic<T>::value>::type* = nullptr>
constexpr T narrow_cast(U u) noexcept {
  const T t = details::NarrowCastImpl<T>(u);

  ONE_HARDENING_ASSERT(!(static_cast<U>(t) != u));

  return t;
}

}  // namespace hcoona
