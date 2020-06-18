load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "hiredis",
    srcs = [
        "async.c",
        "async.h",
        "dict.c",
        "dict.h",
        "fmacros.h",
        "hiredis.c",
        "net.c",
        "net.h",
        "sds.c",
        "sds.h",
    ],
    hdrs = [
        "hiredis.h",
    ],
    copts = [
        "-std=c99",
        "-Wnounused-function",
    ],
    textual_hdrs = ["dict.c"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "hiredis_test",
    srcs = ["test.c"],
    copts = ["-std=c99"],
    deps = [":hiredis"],
)
