genrule(
    name = "config_h",
    srcs = [
        "configure",
        "install-sh",
        "missing",
        "config.guess",
        "config.sub",
        "Makefile.in",
        "config.h.in",
        "ltmain.sh",
    ] + glob([
        "src/**",
        "generated/**",
    ]),
    outs = [
        "config.h",
    ],
    cmd = "pushd external/org_apache_zookeeper" +
          "&& ./configure --without-cppunit " +
          "&& popd " +
          "&& mv external/org_apache_zookeeper/config.h $(location config.h)",
)

cc_library(
    name = "config",
    hdrs = [":config.h"],
)

# TODO(zhangshuai.ustc): Use one flags.
cc_library(
    name = "libzookeeper_mt",
    srcs = [
        "generated/zookeeper.jute.c",
        "src/mt_adaptor.c",
        "src/recordio.c",
        "src/zk_hashtable.c",
        "src/zk_log.c",
        "src/zookeeper.c",
        "src/hashtable/hashtable.c",
        "src/hashtable/hashtable_itr.c",
        "src/zk_adaptor.h",
        "src/zk_hashtable.h",
        "src/hashtable/hashtable.h",
        "src/hashtable/hashtable_itr.h",
        "src/hashtable/hashtable_private.h",
    ] + select({
        "@bazel_tools//src/conditions:windows": [
            "src/winport.h",
            "src/winport.c",
        ],
        "//conditions:default": [],
    }),
    hdrs = [
        "generated/zookeeper.jute.h",
        "include/proto.h",
        "include/recordio.h",
        "include/zookeeper.h",
        "include/zookeeper_log.h",
        "include/zookeeper_version.h",
    ] + select({
        "@bazel_tools//src/conditions:windows": [
            "include/winconfig.h",
        ],
        "//conditions:default": [],
    }),
    defines = [
        "THREADED",
        "USE_STATIC_LIB",
    ] + select({
        "@bazel_tools//src/conditions:windows": [
            "_WINDOWS",
            "WIN32",
        ],
        "//conditions:default": [],
    }),
    includes = [
        ".",
        "generated",
        "include",
        "src",
        "src/hashtable",
    ],
    linkopts = select({
        "@bazel_tools//src/conditions:windows": ["-DEFAULTLIB:ws2_32.lib"],
        "//conditions:default": ["-lpthread"],
    }),
    linkstatic = True,
    visibility = ["//visibility:public"],
    deps = select({
        "@bazel_tools//src/conditions:windows": [
            "@//third_party/zookeeper-client-c:config",
        ],
        "//conditions:default": [
            ":config",
        ],
    }),
)
