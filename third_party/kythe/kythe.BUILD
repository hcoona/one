load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_proto_library")

# Build file for tools integrating Bazel with clang tooling.
package(features = ["layering_check"])

proto_library(
    name = "extra_actions_base_proto",
    srcs = ["third_party/bazel/src/main/protobuf/extra_actions_base.proto"],
)

cc_proto_library(
    name = "extra_actions_base_cc_proto",
    deps = [":extra_actions_base_proto"],
)

cc_library(
    name = "init",
    srcs = [":kythe/cxx/common/init.cc"],
    hdrs = [":kythe/cxx/common/init.h"],
    local_defines = ["KYTHE_OVERRIDE_ASSERT_FAIL"],
    deps = [
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/debugging:failure_signal_handler",
        "@com_google_absl//absl/debugging:symbolize",
    ],
)

# Extracts a single compile command from an extra action.
cc_binary(
    name = "extract_compile_command",
    srcs = ["kythe/cxx/tools/generate_compile_commands/extract_compile_command.cc"],
    deps = [
        ":extra_actions_base_cc_proto",
        ":init",
        "@com_github_google_glog//:glog",
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_protobuf//:protobuf",
    ],
)

action_listener(
    name = "extract_json",
    extra_actions = [":extra_action"],
    mnemonics = ["CppCompile"],
    visibility = ["//visibility:public"],
)

extra_action(
    name = "extra_action",
    cmd = "$(location :extract_compile_command) \
        $(EXTRA_ACTION_FILE) \
        $(output $(ACTION_ID).compile_command.json)",
    out_templates = ["$(ACTION_ID).compile_command.json"],
    tools = [":extract_compile_command"],
)
