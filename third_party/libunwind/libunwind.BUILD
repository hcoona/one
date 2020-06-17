load("@rules_cc//cc:defs.bzl", "cc_library")

# Copied from https://github.com/apache/incubator-heron/blob/eec8be6eaf20c85ed9b1a1801fc02a7ffaeb413b/third_party/libunwind/libunwind.BUILD
package(default_visibility = ["//visibility:public"])

config_setting(
    name = "darwin",
    values = {
        "cpu": "darwin",
    },
    visibility = ["//visibility:public"],
)

genrule(
    name = "libunwind-srcs",
    srcs = glob(["**"]),
    outs = [
        "include/libunwind-common.h",
        "lib/libunwind.a",
        "lib/libunwind-coredump.a",
        "lib/libunwind-ptrace.a",
        "lib/libunwind-setjmp.a",
        "lib/libunwind-x86_64.a",
    ],
    cmd = select({
        ":darwin": "",
        "//conditions:default": "\n".join([
            "export SOURCE_DIR=$$(pwd)",
            "export INSTALL_DIR=$$(pwd)/$(@D)",
            "export TMP_DIR=$$(mktemp -d -t libunwind.XXXXX)",
            "mkdir -p $$TMP_DIR",
            "cp -LR $$(pwd)/external/org_nongnu_libunwind/* $$TMP_DIR",
            "cd $$TMP_DIR",
            "./configure --prefix=$$INSTALL_DIR --enable-shared=no --disable-minidebuginfo --with-pic",
            'make install SUBDIRS="src tests"',
            "rm -rf $$TMP_DIR",
        ]),
    }),
)

cc_library(
    name = "libunwind",
    srcs = select({
        ":darwin": [],
        "//conditions:default": [
            "include/libunwind.h",
            "lib/libunwind.a",
        ],
    }),
    hdrs = select({
        ":darwin": [],
        "//conditions:default": ["include/libunwind.h"],
    }),
    includes = ["include"],
    linkstatic = 1,
)

filegroup(
    name = "libunwind-files",
    srcs = select({
        ":darwin": [],
        "//conditions:default": [
            "include/libunwind.h",
            "include/libunwind-coredump.h",
            "include/libunwind-dynamic.h",
            "include/libunwind-ptrace.h",
            "include/libunwind-x86_64.h",
            "include/unwind.h",
            "lib/libunwind.a",
            "lib/libunwind-coredump.a",
            "lib/libunwind-ptrace.a",
            "lib/libunwind-setjmp.a",
            "lib/libunwind-x86_64.a",
        ],
    }),
)
