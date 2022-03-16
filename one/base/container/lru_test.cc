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

#include "one/base/container/lru.h"

#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "gtest/gtest.h"
#include "tools/cpp/runfiles/runfiles.h"

namespace hcoona {

TEST(TestLru, Case01) {
  std::string error;
  auto runfiles = std::unique_ptr<bazel::tools::cpp::runfiles::Runfiles>(
      bazel::tools::cpp::runfiles::Runfiles::CreateForTest(&error));
  ASSERT_TRUE(runfiles != nullptr) << error;
  const std::string& file_path = runfiles->Rlocation(
      "com_github_hcoona_one/one/base/container/lru_test_case01.txt");

  static constexpr size_t kLruCapacity = 3000;
  Lru<int, int> lru{kLruCapacity};

  std::ifstream is(file_path);
  for (std::string line; std::getline(is, line);) {
    std::vector<std::string_view> v =
        absl::StrSplit(absl::StripAsciiWhitespace(line), ',');
    if (v.empty()) {
      continue;
    }

    if (v.size() == 1) {
      int key;
      ASSERT_TRUE(absl::SimpleAtoi(v[0], &key));
      lru.Get(key);
      continue;
    }

    if (v.size() == 2) {
      int key;
      int value;
      ASSERT_TRUE(absl::SimpleAtoi(v[0], &key));
      ASSERT_TRUE(absl::SimpleAtoi(v[1], &value));
      lru.Put(key, value);
      continue;
    }

    FAIL() << "Should not reach here. size=" << v.size();
  }
}

}  // namespace hcoona
