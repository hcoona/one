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
// This file header defines a generic argument accept C-style string.

#pragma once

#include <string>

namespace hcoona {

class CStringArg {
 public:
  // NOLINTNEXTLINE: by-design non-explicit.
  CStringArg(const char* str) : str_(str) {}

  // NOLINTNEXTLINE: by-design non-explicit.
  CStringArg(const std::string& str) : str_(str.c_str()) {}

  [[nodiscard]] const char* c_str() const { return str_; }

 private:
  const char* str_;
};

}  // namespace hcoona
