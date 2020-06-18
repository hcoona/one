load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library")
load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

copy_file(
    name = "config_h",
    src = "@//third_party/flex:config.h",
    out = "src/config.h",
)

cc_binary(
    name = "flex",
    srcs = [
        "src/parse.c",
        "src/stage1scan.c",
    ],
    visibility = ["//visibility:public"],
    deps = [":common"],
)

cc_library(
    name = "fl",
    srcs = [
        "src/libmain.c",
        "src/libyywrap.c",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "stage1flex",
    srcs = [
        "src/parse.c",
        "src/scan.c",
    ],
    deps = [":common"],
)

genrule(
    name = "gen_stage1scan_c",
    srcs = ["src/scan.l"],
    outs = ["src/stage1scan.c"],
    cmd = "$(location stage1flex) -o $(location src/stage1scan.c) $(location src/scan.l)",
    tools = [":stage1flex"],
)

cc_binary(
    name = "stage1scan",
    srcs = [
        "src/parse.c",
        "src/stage1scan.c",
    ],
    deps = [":common"],
)

cc_library(
    name = "common",
    srcs = [
        "src/buf.c",
        "src/ccl.c",
        "src/dfa.c",
        "src/ecs.c",
        "src/filter.c",
        "src/flexdef.h",
        "src/flexint.h",
        "src/gen.c",
        "src/gettext.h",
        "src/main.c",
        "src/misc.c",
        "src/nfa.c",
        "src/options.c",
        "src/options.h",
        "src/parse.h",
        "src/regex.c",
        "src/scanflags.c",
        "src/scanopt.c",
        "src/scanopt.h",
        "src/skel.c",
        "src/sym.c",
        "src/tables.c",
        "src/tables.h",
        "src/tables_shared.c",
        "src/tables_shared.h",
        "src/tblcmp.c",
        "src/version.h",
        "src/yylex.c",
        ":src/config.h",
    ],
    defines = [
        "HAVE_CONFIG_H",
        "LOCALEDIR=\\\"/usr/local/share/locale\\\"",
    ],
    includes = ["src"],
)
