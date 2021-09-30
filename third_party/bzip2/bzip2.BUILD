load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "bz2",
    srcs = [
        "blocksort.c",
        "bzlib.c",
        "bzlib_private.h",
        "compress.c",
        "crctable.c",
        "decompress.c",
        "huffman.c",
        "randtable.c",
    ],
    hdrs = [
        "bzlib.h",
    ],
    copts = [
        "-Wno-implicit-function-declaration",
        "-Wno-int-conversion",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
)
