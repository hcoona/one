load("@bazel_skylib//rules:copy_file.bzl", "copy_file")

copy_file(
    name = "apr_ldap_h",
    src = "@inf_bmq//third_party/apr_util:apr_ldap.h",
    out = "include/apr_ldap.h",
)

copy_file(
    name = "apu_h",
    src = "@inf_bmq//third_party/apr_util:apu.h",
    out = "include/apu.h",
)

copy_file(
    name = "apu_want_h",
    src = "@inf_bmq//third_party/apr_util:apu_want.h",
    out = "include/apu_want.h",
)

copy_file(
    name = "apu_config_h",
    src = "@inf_bmq//third_party/apr_util:apu_config.h",
    out = "include/private/apu_config.h",
)

copy_file(
    name = "apu_select_dbm_h",
    src = "@inf_bmq//third_party/apr_util:apu_select_dbm.h",
    out = "include/private/apu_select_dbm.h",
)

cc_library(
    name = "apr_util",
    srcs = [
        "buckets/apr_brigade.c",
        "buckets/apr_buckets.c",
        "buckets/apr_buckets_alloc.c",
        "buckets/apr_buckets_eos.c",
        "buckets/apr_buckets_file.c",
        "buckets/apr_buckets_flush.c",
        "buckets/apr_buckets_heap.c",
        "buckets/apr_buckets_mmap.c",
        "buckets/apr_buckets_pipe.c",
        "buckets/apr_buckets_pool.c",
        "buckets/apr_buckets_refcount.c",
        "buckets/apr_buckets_simple.c",
        "buckets/apr_buckets_socket.c",
        "crypto/apr_crypto.c",
        "crypto/apr_md4.c",
        "crypto/apr_md5.c",
        "crypto/apr_passwd.c",
        "crypto/apr_sha1.c",
        "crypto/apr_siphash.c",
        "crypto/crypt_blowfish.c",
        "crypto/getuuid.c",
        "crypto/uuid.c",
        "dbd/apr_dbd.c",
        "dbm/apr_dbm.c",
        "dbm/apr_dbm_sdbm.c",
        "dbm/sdbm/sdbm.c",
        "dbm/sdbm/sdbm_hash.c",
        "dbm/sdbm/sdbm_lock.c",
        "dbm/sdbm/sdbm_pair.c",
        "encoding/apr_base64.c",
        "hooks/apr_hooks.c",
        "ldap/apr_ldap_stub.c",
        "ldap/apr_ldap_url.c",
        "memcache/apr_memcache.c",
        "misc/apr_date.c",
        "misc/apr_queue.c",
        "misc/apr_reslist.c",
        "misc/apr_rmm.c",
        "misc/apr_thread_pool.c",
        "misc/apu_dso.c",
        "misc/apu_version.c",
        "redis/apr_redis.c",
        "strmatch/apr_strmatch.c",
        "uri/apr_uri.c",
        "xlate/xlate.c",
        "xml/apr_xml.c",
        ":apr_ldap_h",
        ":apu_config_h",
        ":apu_h",
        ":apu_select_dbm_h",
        ":apu_want_h",
    ],
    hdrs = glob(["**/*.h"]),
    copts = [
        "-Wno-expansion-to-defined",
    ],
    defines = [
        "_GNU_SOURCE",
        "_REENTRANT",
        "HAVE_CONFIG_H",
        "LINUX",
    ],
    includes = [
        "./include",
        "./include/private",
    ],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
    deps = [
        "@org_apache_apr//:apr",
    ],
)
