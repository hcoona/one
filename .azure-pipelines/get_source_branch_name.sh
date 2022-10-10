#!/usr/bin/env bash
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
