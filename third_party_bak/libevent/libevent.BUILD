load("@rules_cc//cc:defs.bzl", "cc_library")
load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

copy_file(
    name = "config_h",
    src = "@//third_party/libevent:config.h",
    out = "config.h",
)

copy_file(
    name = "evconfig_private_h",
    src = "@//third_party/libevent:evconfig-private.h",
    out = "evconfig-private.h",
)

copy_file(
    name = "event_config_h",
    src = "@//third_party/libevent:event-config.h",
    out = "include/event2/event-config.h",
)

cc_library(
    name = "libevent",
    srcs = [
        "buffer.c",
        "bufferevent.c",
        "bufferevent_filter.c",
        "bufferevent_openssl.c",
        "bufferevent_pair.c",
        "bufferevent_ratelim.c",
        "bufferevent_sock.c",
        "epoll.c",
        "evdns.c",
        "event.c",
        "event_tagging.c",
        "evmap.c",
        "evrpc.c",
        "evthread.c",
        "evthread_pthread.c",
        "evutil.c",
        "evutil_rand.c",
        "evutil_time.c",
        "http.c",
        "listener.c",
        "log.c",
        "poll.c",
        "select.c",
        "signal.c",
        "strlcpy.c",
        ":config.h",
        ":evconfig-private.h",
        ":include/event2/event-config.h",
    ],
    hdrs = glob(["**/*.h"]),
    copts = [
        "-fno-strict-aliasing",
        "-W",
        "-Waddress",
        "-Wall",
        "-Wbad-function-cast",
        "-Wdeclaration-after-statement",
        "-Wfloat-equal",
        "-Winit-self",
        "-Wmissing-declarations",
        "-Wmissing-field-initializers",
        "-Wmissing-prototypes",
        "-Wnested-externs",
        "-Wno-unused-function",
        "-Wno-unused-parameter",
        "-Wpointer-arith",
        "-Wredundant-decls",
        "-Wstrict-aliasing",
        "-Wstrict-prototypes",
        "-Wundef",
        "-Wwrite-strings",
    ],
    defines = [
        "HAVE_CONFIG_H",
    ],
    includes = [
        "./compat",
        "./include",
    ],
    linkopts = [
        "-pthread",
    ],
    textual_hdrs = [
        "arc4random.c",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "@boringssl//:ssl",
        "@com_github_madler_zlib//:zlib",
    ],
)
