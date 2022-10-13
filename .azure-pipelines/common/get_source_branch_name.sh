#!/usr/bin/env bash
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
#
# Get branch name from either Build.SourceBranch or System.PullRequest.SourceBranch
#
# See
# 1. https://stackoverflow.com/a/59958130/427652
# 2. https://learn.microsoft.com/en-us/azure/devops/pipelines/build/variables?view=azure-devops&tabs=yaml
# 3. https://learn.microsoft.com/en-us/azure/devops/pipelines/process/set-variables-scripts?view=azure-devops&tabs=bash
#
# See https://www.gnu.org/software/bash/manual/html_node/The-Set-Builtin.html
# for shell options meanings.
set -Eeux -o pipefail

#########################
# Entry point
#########################

if [ "$BUILD_REASON" == "PullRequest" ]; then
  # shellcheck disable=SC2001
  BRANCH_NAME="$(echo "$SYSTEM_PULLREQUEST_SOURCEBRANCH" | sed 's#^refs/heads/##')"
else
  # shellcheck disable=SC2001
  BRANCH_NAME="$(echo "$BUILD_SOURCEBRANCH" | sed 's#^refs/heads/##')"
fi
echo "##vso[task.setvariable variable=BuildBuddy.SourceBranchName;]$BRANCH_NAME"
