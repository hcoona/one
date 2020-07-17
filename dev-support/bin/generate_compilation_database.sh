#!/usr/bin/env bash

# Generates a compile_commands.json file at $(bazel info execution_root) for
# your Clang tooling needs.
# FYI: https://github.com/kythe/kythe/blob/master/tools/cpp/generate_compilation_database.sh

set -Eeuxo pipefail
SCRIPT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
BD3_ROOT="$(realpath "${SCRIPT_ROOT}"/../../)"
BAZELIST_EXEC="bazel"

pushd "$BD3_ROOT" >/dev/null

$BAZELIST_EXEC --bazelrc=/dev/null build \
  --keep_going \
  --strict_system_includes \
  --experimental_action_listener=@io_kythe//kythe/cxx/tools/generate_compile_commands:extract_json \
  --noshow_progress \
  --noshow_loading_progress \
  $($BAZELIST_EXEC --bazelrc=/dev/null query 'kind(cc_.*, //...) - attr(tags, manual, //...)') >/dev/null

pushd "$($BAZELIST_EXEC info execution_root)" >/dev/null
echo "[" >compile_commands.json
find . -name '*.compile_command.json' -exec bash -c 'cat {} && echo ,' \; >>compile_commands.json
sed -i 's/\/sandbox\/linux-sandbox\/[[:digit:]]\+//' compile_commands.json
sed -i '$s/,$//' compile_commands.json
echo "]" >>compile_commands.json
popd >/dev/null # "$($BAZELIST_EXEC info execution_root)"

popd >/dev/null # "$BD3_ROOT"

# TODO: Fix the base directory in compile_command.json

# cp "$($BAZELIST_EXEC info execution_root)/compile_commands.json" .
