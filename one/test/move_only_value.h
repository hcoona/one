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

#pragma once

#include <utility>

namespace hcoona {

template <typename T>
class MoveOnlyValue {
 public:
  MoveOnlyValue() = default;
  constexpr explicit MoveOnlyValue(T value) : value_(std::move(value)) {}
  ~MoveOnlyValue() = default;

  // Disallow copy.
  constexpr MoveOnlyValue(const MoveOnlyValue&) = delete;
  constexpr MoveOnlyValue& operator=(const MoveOnlyValue&) = delete;

  // Allow move.
  constexpr MoveOnlyValue(MoveOnlyValue&&) noexcept = default;
  constexpr MoveOnlyValue& operator=(MoveOnlyValue&&) noexcept = default;

  explicit operator T() const { return value_; }

  constexpr const T& value() const& { return value_; }
  constexpr T& value() & { return value_; }
  constexpr T&& value() && { return std::move(value_); }

 private:
  T value_;
};

}  // namespace hcoona
