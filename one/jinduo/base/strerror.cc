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
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/base/strerror.h"

#include <string.h>  // NOLINT(modernize-deprecated-headers, hicpp-deprecated-headers)

namespace jinduo {

namespace {

constexpr size_t kErrnoBufferSize = 512;

__thread char t_errnobuf[kErrnoBufferSize];

}  // namespace

const char* strerror_tl(int saved_errno) {
  return strerror_r(saved_errno, t_errnobuf, sizeof(t_errnobuf));
}

}  // namespace jinduo
