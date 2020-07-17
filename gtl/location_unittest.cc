// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/location.h"

#include "gtl/compiler_specific.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace gtl {

namespace {

// This is a typical use: taking Location::Current as a default parameter.
// So even though this looks contrived, it confirms that such usage works as
// expected.
Location WhereAmI(const Location& location = Location::Current()) {
  return location;
}

}  // namespace

TEST(LocationTest, CurrentYieldsCorrectValue) {
  int previous_line = __LINE__;
  Location here = WhereAmI();
  EXPECT_NE(here.program_counter(), WhereAmI().program_counter());
#if SUPPORTS_LOCATION_BUILTINS
  EXPECT_THAT(here.file_name(), ::testing::EndsWith("location_unittest.cc"));
#if BUILDFLAG(ENABLE_LOCATION_SOURCE)
  EXPECT_EQ(here.line_number(), previous_line + 1);
  EXPECT_STREQ("TestBody", here.function_name());
#endif
#elif defined(OFFICIAL_BUILD)
#error Location builtins must be supported in official builds.
#elif BUILDFLAG(FROM_HERE_USES_LOCATION_BUILTINS)
#error FROM_HERE requires location builtins to be supported.
#endif
  ALLOW_UNUSED_LOCAL(previous_line);
}

}  // namespace gtl