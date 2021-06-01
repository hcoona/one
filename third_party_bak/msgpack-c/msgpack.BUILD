load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

cc_library(
    name = "msgpack",
    srcs = [
        "src/object.cpp",
        "src/objectc.c",
        "src/unpack.c",
        "src/version.c",
        "src/vrefbuffer.c",
        "src/zone.c",
    ],
    hdrs = [
        "src/msgpack.h",
        "src/msgpack.hpp",
        "src/msgpack/fbuffer.h",
        "src/msgpack/fbuffer.hpp",
        "src/msgpack/object.h",
        "src/msgpack/object.hpp",
        "src/msgpack/pack.h",
        "src/msgpack/pack.hpp",
        "src/msgpack/pack_define.h",
        "src/msgpack/pack_template.h",
        "src/msgpack/sbuffer.h",
        "src/msgpack/sbuffer.hpp",
        "src/msgpack/sysdep.h",
        "src/msgpack/type.hpp",
        "src/msgpack/type/bool.hpp",
        "src/msgpack/type/define.hpp",
        "src/msgpack/type/deque.hpp",
        "src/msgpack/type/fixint.hpp",
        "src/msgpack/type/float.hpp",
        "src/msgpack/type/int.hpp",
        "src/msgpack/type/list.hpp",
        "src/msgpack/type/map.hpp",
        "src/msgpack/type/nil.hpp",
        "src/msgpack/type/pair.hpp",
        "src/msgpack/type/raw.hpp",
        "src/msgpack/type/set.hpp",
        "src/msgpack/type/string.hpp",
        "src/msgpack/type/tr1/unordered_map.hpp",
        "src/msgpack/type/tr1/unordered_set.hpp",
        "src/msgpack/type/tuple.hpp",
        "src/msgpack/type/vector.hpp",
        "src/msgpack/unpack.h",
        "src/msgpack/unpack.hpp",
        "src/msgpack/unpack_define.h",
        "src/msgpack/unpack_template.h",
        "src/msgpack/version.h",
        "src/msgpack/vrefbuffer.h",
        "src/msgpack/vrefbuffer.hpp",
        "src/msgpack/zbuffer.h",
        "src/msgpack/zbuffer.hpp",
        "src/msgpack/zone.h",
        "src/msgpack/zone.hpp",
    ],
    copts = [
        "-fPIC",
    ],
    defines = [
        "msgpack_EXPORTS",
        "PIC",
    ],
    strip_include_prefix = "./src",
    visibility = ["//visibility:public"],
    deps = ["@com_github_madler_zlib//:zlib"],
)

cc_test(
    name = "test_buffer",
    srcs = ["test/buffer.cc"],
    deps = [
        ":msgpack",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "test_cases",
    srcs = ["test/cases.cc"],
    deps = [
        ":msgpack",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "test_msgpack",
    srcs = ["test/msgpack_test.cpp"],
    deps = [
        ":msgpack",
        "@com_google_googletest//:gtest_main",
    ],
)
