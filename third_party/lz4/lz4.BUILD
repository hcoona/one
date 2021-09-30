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
