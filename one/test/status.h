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
// ONE_PREDICT_TRUE & ONE_PREDICT_FALSE are coming from
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

#include "gtest/gtest.h"

namespace hcoona {
namespace test {

template <typename Status>
::testing::AssertionResult AssertStatusOk(Status&& s) {
  if (!s.ok()) {
    return ::testing::AssertionFailure() << s;
  }

  return ::testing::AssertionSuccess();
}

}  // namespace test
}  // namespace hcoona
