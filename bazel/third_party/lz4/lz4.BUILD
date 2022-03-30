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

load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")

cc_library(
    name = "lz4",
    srcs = [
        "lib/lz4.c",
        "lib/lz4frame.c",
        "lib/lz4hc.c",
        "lib/xxhash.c",
        "lib/xxhash.h",
    ],
    hdrs = [
        "lib/lz4.c",
        "lib/lz4.h",
        "lib/lz4frame.h",
        "lib/lz4frame_static.h",
        "lib/lz4hc.h",
    ],
    includes = ["./lib"],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "fullbench",
    srcs = [
        "programs/platform.h",
        "programs/util.h",
        "tests/fullbench.c",
    ],
    includes = ["./programs"],
    deps = [":lz4"],
)
