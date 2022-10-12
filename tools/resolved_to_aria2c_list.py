#!/usr/bin/env python3
#
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
"""Convert bazel generated resolved.bzl to aria2c download file list to
generate distdir files.

To prepare a distdir for Bazel offline build, you need to generate a
`resolved.bzl` file firstly.

  bazel sync --experimental_repository_resolved_file=resolved.bzl

Then you need to cook the file into a list to feed aria2c for bulk downloading.
The simple grep & awk is not working here because we need to deduplicate the
files from URLs & we need to assign output file name to avoid aria2c
automatically rename the file name according to HTTP response suggestion.

Use the aria2 command to avoid download existing files

  aria2c -i distdir_file.txt --auto-file-renaming=false --conditional-get=true

You still need to follow this guide to generate bazel internal distdir

  https://bazel.build/run/build#running-bazel-airgapped
"""

import argparse
import ast
import logging
from urllib.parse import urlparse


def init_argparse() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        usage="%(prog)s [OPTION] <resolved_bzl_file> <distdir_file>",
        description=__doc__,
    )
    parser.add_argument("-v",
                        "--version",
                        action="version",
                        version=f"{parser.prog} version 1.0.0")
    parser.add_argument("resolved_bzl_file")
    parser.add_argument("distdir_file")
    return parser


def run(resolved_bzl_file, file_list):
    logging.info("Converting %s to %s...", resolved_bzl_file, file_list)
    with open(resolved_bzl_file, 'rt') as file_pointer:
        tree = ast.parse(file_pointer.read())
    resolved = ast.literal_eval(tree.body[0].value)
    logging.debug("resolved=%s", resolved)

    with open(file_list, "wt") as file_pointer:
        for item in filter(
                lambda item: item["original_rule_class"].endswith("http_archive"
                                                                 ), resolved):
            if len(item["repositories"]) != 1:
                logging.fatal("rule repositories count(%d) != 1. %s",
                              len(item["repositories"]),
                              item["original_attributes"])

            rule_attributes = item["repositories"][0]["attributes"]

            file_sha256 = rule_attributes["sha256"]
            if not file_sha256:
                logging.warning("file has no sha256, skipped. %s",
                                item["original_attributes"])
                continue

            if rule_attributes["url"]:
                file_urls = [rule_attributes["url"]]
            else:
                file_urls = rule_attributes["urls"]
            if len(file_urls) == 0:
                logging.fatal("file url & urls are empty. %s",
                              item["original_attributes"])

            file_filename = urlparse(file_urls[0]).path.rsplit('/', 1)[-1]

            msg = "%s\n  out=%s\n  checksum=sha-256=%s" % (
                "\t".join(file_urls), file_filename, file_sha256)
            file_pointer.write(msg)
            file_pointer.write("\n")
            logging.info(msg)


if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    parser = init_argparse()
    args = parser.parse_args()
    run(args.resolved_bzl_file, args.distdir_file)
