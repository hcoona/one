# Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
# All rights reserved.
#
# This file is part of ONE.
#
# ONE is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# ONE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# ONE. If not, see <https://www.gnu.org/licenses/>.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "fmt",
    srcs = [
        "src/format.cc",
        "src/os.cc",
    ],
    hdrs = glob(["include/**/*.h"]),
    copts = [
        "-pedantic-errors",
        "-pedantic",
    ],
    defines = [
        "FMT_LOCALE",
        "FMT_SAFE_DURATION_CAST",
    ],
    strip_include_prefix = "include",
    visibility = ["//visibility:public"],
)
