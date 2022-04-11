#! /usr/bin/env bash
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

set -Eeuo pipefail

# --- begin runfiles.bash initialization v2 ---
# Copy-pasted from the Bazel Bash runfiles library v2.
# https://github.com/bazelbuild/bazel/blob/5.1.1/tools/bash/runfiles/runfiles.bash
set -uo pipefail
f=bazel_tools/tools/bash/runfiles/runfiles.bash
# shellcheck source=/dev/null
source "${RUNFILES_DIR:-/dev/null}/$f" 2>/dev/null ||
  source "$(grep -sm1 "^$f " "${RUNFILES_MANIFEST_FILE:-/dev/null}" | cut -f2- -d' ')" 2>/dev/null ||
  source "$0.runfiles/$f" 2>/dev/null ||
  source "$(grep -sm1 "^$f " "$0.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null ||
  source "$(grep -sm1 "^$f " "$0.exe.runfiles_manifest" | cut -f2- -d' ')" 2>/dev/null ||
  {
    echo >&2 "ERROR: cannot find $f"
    exit 1
  }
f=
set -e
# --- end runfiles.bash initialization v2 ---

GLIBC_VERSION="$(eval "$(rlocation com_github_hcoona_one/bazel/build/glibc_version)")"
LESS_GLIBC_VERSION="$(printf "%s\n%s\n" "$GLIBC_VERSION" "$1" | sort --version-sort | head -n 1)"
if [ "$LESS_GLIBC_VERSION" = "$1" ]; then
  echo 1
else
  echo 0
fi
