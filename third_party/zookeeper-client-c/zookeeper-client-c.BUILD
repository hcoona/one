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

# Use `#include "zookeeper.h"` directly.
cc_library(
    name = "libzookeeper_mt",
    srcs = [
        "config.h",
        "generated/zookeeper.jute.c",
        "src/hashtable/hashtable.c",
        "src/hashtable/hashtable.h",
        "src/hashtable/hashtable_itr.c",
        "src/hashtable/hashtable_itr.h",
        "src/hashtable/hashtable_private.h",
        "src/mt_adaptor.c",
        "src/recordio.c",
        "src/zk_adaptor.h",
        "src/zk_hashtable.c",
        "src/zk_hashtable.h",
        "src/zk_log.c",
        "src/zookeeper.c",
    ],
    hdrs = [
        "generated/zookeeper.jute.h",
        "include/proto.h",
        "include/recordio.h",
        "include/zookeeper.h",
        "include/zookeeper_log.h",
        "include/zookeeper_version.h",
    ],
    defines = [
        "THREADED",
    ],
    includes = [
        ".",
        "generated",
        "include",
        "src",
        "src/hashtable",
    ],
    linkopts = [
        "-lpthread",
    ],
    visibility = ["//visibility:public"],
)

cc_binary(
    name = "cli_mt",
    srcs = ["src/cli.c"],
    defines = ["THREADED"],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
    deps = [":libzookeeper_mt"],
)
