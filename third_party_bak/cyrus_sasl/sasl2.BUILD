load("@rules_cc//cc:defs.bzl", "cc_library")
load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

copy_file(
    name = "config_h",
    src = "@//third_party/cyrus_sasl:config.h",
    out = "config.h",
)

cc_library(
    name = "sasl2",
    srcs = [
        "saslauthd/auth_dce.c",
        "saslauthd/auth_getpwent.c",
        "saslauthd/auth_httpform.c",
        "saslauthd/auth_krb4.c",
        "saslauthd/auth_krb5.c",
        "saslauthd/auth_ldap.c",
        "saslauthd/auth_pam.c",
        "saslauthd/auth_rimap.c",
        "saslauthd/auth_sasldb.c",
        "saslauthd/auth_shadow.c",
        "saslauthd/auth_sia.c",
        "saslauthd/cache.c",
        "saslauthd/cfile.c",
        "saslauthd/krbtf.c",
        "saslauthd/lak.c",
        "saslauthd/md5.c",
        "saslauthd/mechanisms.c",
        "saslauthd/utils.c",
    ],
    hdrs = [
        "config_h",
    ] + glob(["**/*.h"]),
    defines = [
        "HAVE_CONFIG_H",
        "OBSOLETE_CRAM_ATTR=1",
        "OBSOLETE_DIGEST_ATTR=1",
        "SASLAUTHD_CONF_FILE_DEFAULT=\"\\\"/usr/local/etc/saslauthd.conf\\\"\"",
    ],
    includes = ["./include"],
    linkopts = [
        "-ldl",
        "-lresolv",
    ],
    visibility = ["//visibility:public"],
    deps = ["@boringssl//:crypto"],
)
