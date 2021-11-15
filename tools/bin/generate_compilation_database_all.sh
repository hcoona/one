#!/bin/bash

# Generates a compile_commands.json file at $(bazel info execution_root) for
# the given file path.

set -ex

bazel build "$@" \
  --experimental_action_listener=@io_kythe//:extract_json \
  --noshow_progress \
  --noshow_loading_progress \
  --output_groups=compilation_outputs \
  //... >/dev/null

set +x

BAZEL_ROOT="$(bazel info execution_root)"
pushd "$BAZEL_ROOT" >/dev/null
find . -name '*.compile_command.json' -print0 | while read -r -d '' fname; do
  sed -e "s|@BAZEL_ROOT@|$BAZEL_ROOT|g" <"$fname" >>compile_commands.json
  echo "" >>compile_commands.json
done
# Decompose, insert and keep the most recent entry for a given file, then
# recombine.
sed 's/\(^[[]\)\|\([],]$\)//;/^$/d;' <compile_commands.json |
  tac | sort -u -t, -k1,1 |
  sed '1s/^./[\0/;s/}$/},/;$s/,$/]/' >compile_commands.json.tmp
mv compile_commands.json{.tmp,}
popd >/dev/null

BAZEL_WORKSPACE="$(bazel info workspace)"
# "$BAZEL_WORKSPACE/bazel-bin/codelab/compdb_purger/main" \
#   --logtostderr \
#   --input "$BAZEL_ROOT/compile_commands.json" \
#   --output "$BAZEL_WORKSPACE/compile_commands.json"

# cp -f "$BAZEL_ROOT/compile_commands.json" "$BAZEL_WORKSPACE/compile_commands.json"
# sed -i 's/ external/ bazel-one\/external/g' "$BAZEL_WORKSPACE/compile_commands.json"
# sed -i 's/\"external/\"bazel-one\/external/g' "$BAZEL_WORKSPACE/compile_commands.json"

rm -rf "$BAZEL_WORKSPACE/.bazel-one"
mkdir -p "$BAZEL_WORKSPACE/.bazel-one"
cp -r "$BAZEL_WORKSPACE/bazel-one/"* "$BAZEL_WORKSPACE/.bazel-one/."
sed -i -e "s|\"--compile-commands-dir=.*\"|\"--compile-commands-dir=$$BAZEL_WORKSPACE/.bazel-one\"|g" "$BAZEL_WORKSPACE/.vscode/settings.json"
