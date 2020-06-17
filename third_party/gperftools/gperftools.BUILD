load("@rules_cc//cc:defs.bzl", "cc_library")

# Copied from https://github.com/apache/incubator-heron/blob/eec8be6eaf20c85ed9b1a1801fc02a7ffaeb413b/third_party/gperftools/gperftools.BUILD
package(default_visibility = ["//visibility:public"])

config_setting(
    name = "darwin",
    values = {
        "cpu": "darwin",
    },
    visibility = ["//visibility:public"],
)

config_setting(
    name = "k8",
    values = {
        "cpu": "k8",
    },
    visibility = ["//visibility:public"],
)

common_script = [
    "export UNWIND_DIR=$$(pwd)/$(GENDIR)/external/org_nongnu_libunwind",
    "echo $$UNWIND_DIR",
    "export INSTALL_DIR=$$(pwd)/$(@D)",
    "export TMP_DIR=$$(mktemp -d -t gperftools.XXXXX)",
    "mkdir -p $$TMP_DIR",
    "cp -R $$(pwd)/external/com_github_gperftools_gperftools/* $$TMP_DIR",
    "cd $$TMP_DIR",
]

mac_script = "\n".join(common_script + [
    "./configure --prefix=$$INSTALL_DIR --enable-shared=no",
    "make install",
    "rm -rf $$TMP_DIR",
])

linux_script = "\n".join(common_script + [
    './configure --prefix=$$INSTALL_DIR --enable-shared=no --enable-libunwind --with-pic CPPFLAGS=-I$$UNWIND_DIR/include LDFLAGS="-L$$UNWIND_DIR/lib -lunwind"',
    'make install CPPFLAGS=-I$$UNWIND_DIR/include LDFLAGS="-L$$UNWIND_DIR/lib -lunwind"',
    "rm -rf $$TMP_DIR",
])

genrule(
    name = "gperftools-srcs",
    srcs = glob(["**"]) + select({
        ":darwin": [],
        "//conditions:default": ["@org_nongnu_libunwind//:libunwind-files"],
    }),
    outs = [
        "bin/pprof",
        "include/google/heap-checker.h",
        "include/google/heap-profiler.h",
        "include/google/malloc_extension.h",
        "include/google/malloc_extension_c.h",
        "include/google/malloc_hook.h",
        "include/google/malloc_hook_c.h",
        "include/google/profiler.h",
        "include/google/stacktrace.h",
        "include/google/tcmalloc.h",
        "include/gperftools/heap-checker.h",
        "include/gperftools/heap-profiler.h",
        "include/gperftools/malloc_extension.h",
        "include/gperftools/malloc_extension_c.h",
        "include/gperftools/malloc_hook.h",
        "include/gperftools/malloc_hook_c.h",
        "include/gperftools/profiler.h",
        "include/gperftools/stacktrace.h",
        "include/gperftools/tcmalloc.h",
        "lib/libprofiler.a",
        "lib/libtcmalloc.a",
        "lib/libtcmalloc_and_profiler.a",
        "lib/libtcmalloc_debug.a",
        "lib/libtcmalloc_minimal.a",
        "lib/libtcmalloc_minimal_debug.a",
    ],
    cmd = select({
        ":darwin": mac_script,
        "//conditions:default": linux_script,
    }),
)

cc_library(
    name = "tcmalloc",
    srcs = ["lib/libtcmalloc.a"],
    hdrs = [
        "include/google/malloc_extension.h",
        "include/gperftools/malloc_extension.h",
        "include/gperftools/tcmalloc.h",
    ],
    includes = [
        "include",
    ],
    linkstatic = 1,
)

cc_library(
    name = "profiler",
    srcs = ["lib/libprofiler.a"],
    hdrs = [
        "include/gperftools/profiler.h",
    ],
    includes = ["include"],
    linkstatic = 1,
)

cc_library(
    name = "libtcmalloc_and_profiler",
    srcs = ["lib/libtcmalloc_and_profiler.a"],
    hdrs = [
        "include/google/heap-checker.h",
        "include/google/heap-profiler.h",
        "include/google/malloc_extension.h",
        "include/google/malloc_extension_c.h",
        "include/google/malloc_hook.h",
        "include/google/malloc_hook_c.h",
        "include/google/profiler.h",
        "include/google/stacktrace.h",
        "include/google/tcmalloc.h",
        "include/gperftools/heap-checker.h",
        "include/gperftools/heap-profiler.h",
        "include/gperftools/malloc_extension.h",
        "include/gperftools/malloc_extension_c.h",
        "include/gperftools/malloc_hook.h",
        "include/gperftools/malloc_hook_c.h",
        "include/gperftools/profiler.h",
        "include/gperftools/stacktrace.h",
        "include/gperftools/tcmalloc.h",
    ],
    includes = ["include"],
    linkstatic = 1,
    deps = [
        "@org_nongnu_libunwind//:libunwind",
    ],
)

filegroup(
    name = "gperftools-files",
    srcs = [
        "bin/pprof",
        ":profiler",
        ":tcmalloc",
    ],
)
