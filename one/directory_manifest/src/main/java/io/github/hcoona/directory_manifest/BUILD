load("@rules_java//java:defs.bzl", "java_binary", "java_library")

package(
    default_visibility = ["//one/directory_manifest/src:__subpackages__"],
)

java_binary(
    name = "directory_manifest",
    srcs = [
        "App.java",
    ],
    main_class = "io.github.hcoona.directory_manifest.App",
    runtime_deps = [
        "@maven//:ch_qos_logback_logback_classic",
    ],
    deps = [
        ":directory_manifest_controller",
        "@maven//:org_slf4j_slf4j_api",
    ],
)

java_library(
    name = "directory_manifest_controller",
    srcs = [
        "Controller.java",
    ],
    deps = [
        ":log_error_scheduled_thread_pool_executor",
        "@maven//:com_google_guava_guava",
        "@maven//:commons_codec_commons_codec",
        "@maven//:org_slf4j_slf4j_api",
    ],
)

java_library(
    name = "log_error_scheduled_thread_pool_executor",
    srcs = [
        "LogErrorScheduledThreadPoolExecutor.java",
    ],
    deps = [
        "@maven//:org_slf4j_slf4j_api",
    ],
)
