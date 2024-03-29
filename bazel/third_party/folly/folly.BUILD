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

genrule(
    name = "folly_config_h",
    srcs = [
        "CMake/folly-config.h.cmake",
    ],
    outs = [
        "folly/folly-config.h",
    ],
    cmd = ("sed " +
           "-e 's/#cmakedefine FOLLY_USE_JEMALLOC 1/#undef FOLLY_USE_JEMALLOC/' " +
           "-e 's/#cmakedefine FOLLY_HAVE_EXTRANDOM_SFMT19937 1/#undef FOLLY_HAVE_EXTRANDOM_SFMT19937/' " +
           "-e 's/#cmakedefine FOLLY_HAVE_LIBUNWIND 1/#undef FOLLY_HAVE_LIBUNWIND/' " +
           "-e 's/#cmakedefine FOLLY_HAVE_DWARF 1/#undef FOLLY_HAVE_DWARF/' " +
           "-e 's/#cmakedefine FOLLY_HAVE_ELF 1/#undef FOLLY_HAVE_ELF/' " +
           "-e 's/#cmakedefine01 FOLLY_HAVE_GETRANDOM/#define FOLLY_HAVE_GETRANDOM 1/' " +
           # TODO(zhangshuai.ds): set sanitizer options according to config settings.
           "-e 's/#cmakedefine01 FOLLY_LIBRARY_SANITIZE_ADDRESS/#define FOLLY_LIBRARY_SANITIZE_ADDRESS 0/' " +
           "-e 's/#cmakedefine01 FOLLY_HAVE_LIBRT/#define FOLLY_HAVE_LIBRT 1/' " +
           "-e 's/@FOLLY_GFLAGS_NAMESPACE@/gflags/' " +
           "-e 's/#cmakedefine /#define /g' " +
           "$< >$@"),
)

cc_library(
    name = "folly",
    # `folly_base_files` in `CMakeLists.txt`
    # `files`
    srcs = glob(
        [
            "folly/**/*.cpp",
        ],
        exclude = [
            "folly/build/**",
            "folly/logging/example/**",
            "folly/**/*test/**",
            "folly/tools/**",
            "folly/**/*Benchmark.cpp",
            "folly/**/*Test.cpp",
            "folly/experimental/JSONSchemaTester.cpp",
            "folly/experimental/io/HugePageUtil.cpp",
            "folly/python/error.cpp",
            "folly/python/executor.cpp",
            "folly/python/fibers.cpp",
            "folly/python/GILAwareManualExecutor.cpp",
            "folly/experimental/symbolizer/Addr2Line.cpp",
        ],
    ),
    # `hfiles`
    hdrs = glob(
        [
            "folly/**/*.h",
        ],
        exclude = [
            "folly/build/**",
            "folly/logging/example/**",
            "folly/**/*test/**",
            "folly/tools/**",
            "folly/python/fibers.h",
            "folly/python/GILAwareManualExecutor.h",
        ],
    ) + ["folly/folly-config.h"],
    copts = [
        "-Wno-nullability-completeness",
    ],
    linkopts = [
        "-pthread",
        "-ldl",
        "-lrt",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@boost//:algorithm",
        "@boost//:container",
        "@boost//:context",
        "@boost//:crc",
        "@boost//:filesystem",
        "@boost//:intrusive",
        "@boost//:multi_index",
        "@boost//:operators",
        "@boost//:preprocessor",
        "@boost//:program_options",
        "@boost//:regex",
        "@boost//:variant",
        "@com_github_facebook_zstd//:zstd",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_google_double_conversion//:double-conversion",
        "@com_github_google_glog//:glog",
        "@com_github_google_snappy//:snappy",
        "@com_github_jedisct1_libsodium//:sodium",
        "@com_github_libevent_libevent//:event",
        "@com_github_lz4_lz4//:lz4",
        "@openssl//:crypto",
        "@openssl//:ssl",
    ],
)

cc_library(
    name = "folly_test_common",
    testonly = True,
    srcs = [
        "folly/io/async/test/ScopedBoundPort.cpp",
        "folly/io/async/test/SocketPair.cpp",
        "folly/io/async/test/TimeUtil.cpp",
    ],
    hdrs = [
        "folly/container/test/F14TestUtil.h",
        "folly/container/test/TrackingTypes.h",
        "folly/io/async/test/AsyncSSLSocketTest.h",
        "folly/io/async/test/AsyncSocketTest.h",
        "folly/io/async/test/AsyncSocketTest2.h",
        "folly/io/async/test/BlockingSocket.h",
        "folly/io/async/test/MockAsyncSSLSocket.h",
        "folly/io/async/test/MockAsyncServerSocket.h",
        "folly/io/async/test/MockAsyncSocket.h",
        "folly/io/async/test/MockAsyncTransport.h",
        "folly/io/async/test/MockAsyncUDPSocket.h",
        "folly/io/async/test/MockTimeoutManager.h",
        "folly/io/async/test/ScopedBoundPort.h",
        "folly/io/async/test/SocketPair.h",
        "folly/io/async/test/TestSSLServer.h",
        "folly/io/async/test/TimeUtil.h",
        "folly/io/async/test/UndelayedDestruction.h",
        "folly/io/async/test/Util.h",
        "folly/synchronization/test/Semaphore.h",
        "folly/test/DeterministicSchedule.h",
        "folly/test/JsonTestUtil.h",
        "folly/test/TestUtils.h",
    ],
    deps = [
        ":folly",
    ],
)
