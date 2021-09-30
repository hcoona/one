load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "utf8proc",
    srcs = ["utf8proc.c"],
    hdrs = ["utf8proc.h"],
    copts = [
        "-Wall",
        "-Wextra",
        "-pedantic",
    ],
    textual_hdrs = ["utf8proc_data.c"],
    visibility = ["//visibility:public"],
)
