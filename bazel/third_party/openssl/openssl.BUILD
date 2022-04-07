# Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
# All rights reserved.
#
# This file is part of ONE.
#
# ONE is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# ONE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# ONE. If not, see <https://www.gnu.org/licenses/>.

load("@rules_foreign_cc//foreign_cc:defs.bzl", "configure_make")

filegroup(
    name = "all_srcs",
    srcs = glob(["**"]),
)

alias(
    name = "ssl",
    actual = "openssl",
    visibility = ["//visibility:public"],
)

alias(
    name = "crypto",
    actual = "openssl",
    visibility = ["//visibility:public"],
)

# https://wiki.openssl.org/index.php/Compilation_and_Installation
configure_make(
    name = "openssl",
    configure_command = "config",
    configure_in_place = False,
    configure_options = [
        "--api=1.1.0",
        "--openssldir=/usr/lib/ssl",
        "no-idea",
        "no-mdc2",
        "no-rc5",
        "no-ssl3",
        "no-ssl3-method",
        "enable-rfc3779",
        "enable-cms",
        "no-capieng",
        "enable-ec_nistp_64_gcc_128",
        "no-zlib",
        # https://stackoverflow.com/questions/36220341/struct-in6-addr-has-no-member-named-s6-addr32-with-ansi
        "-D_DEFAULT_SOURCE=1",
        "-DPEDANTIC",
    ],
    env = select({
        "@platforms//os:macos": {
            "AR": "",
            "PERL": "$$EXT_BUILD_ROOT$$/$(PERL)",
        },
        "//conditions:default": {
            "PERL": "$$EXT_BUILD_ROOT$$/$(PERL)",
        },
    }),
    lib_name = "openssl",
    lib_source = ":all_srcs",
    # Note that for Linux builds, libssl must come before libcrypto on the linker command-line.
    # As such, libssl must be listed before libcrypto
    out_static_libs = [
        "libssl.a",
        "libcrypto.a",
    ],
    targets = [
        "build_libs",
        "install_dev",
    ],
    toolchains = ["@rules_perl//:current_toolchain"],
    visibility = ["//visibility:public"],
)

filegroup(
    name = "gen_dir",
    srcs = [":openssl"],
    output_group = "gen_dir",
)
