// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.

#include <memory>
#include <string>
#include <vector>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/base/filesystem/posix_file_system.h"
#include "one/cxx_include_graph/compile_commands.h"

bool ValidateStringNotEmpty(const char* /*flag_name*/,
                            const std::string& value) {
  return !value.empty();
}

DEFINE_string(compdb, "", "The path of `compile_commands.json`.");
DEFINE_validator(compdb, &ValidateStringNotEmpty);

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  hcoona::PosixFileSystem file_system;

  std::unique_ptr<hcoona::ReadOnlyMemoryRegion> read_only_memory_region;
  absl::Status s = file_system.NewReadOnlyMemoryRegionFromFile(
      FLAGS_compdb, &read_only_memory_region);
  if (!s.ok()) {
    LOG(ERROR) << s;
    return 1;
  }

  std::vector<hcoona::CompileCommandsItem> compile_commands;
  s = hcoona::Parse(
      reinterpret_cast<const char*>(read_only_memory_region->data()),
      &compile_commands);
  if (!s.ok()) {
    LOG(ERROR) << s;
    return 1;
  }

  LOG(INFO) << "compile_commands.size() = " << compile_commands.size();

  for (const hcoona::CompileCommandsItem& item : compile_commands) {
    LOG(INFO) << item.command;
    LOG(INFO) << item.file;
  }

  // TODO(zhangshuai.ds): Filter none source code items. com_grail_bazel_compdb
  // would generate hdrs & textural_hdrs into compile_commands.json.
  //
  // TODO(zhangshuai.ds): Modify the commands to remove unecessary options.
  //
  // TODO(zhangshuai.ds): Generate including results via -E -MM -MF
  // <output_file>.
  //
  // TODO(zhangshuai.ds): Consider -H option for printing direct includes.
  //
  // TODO(zhangshuai.ds): consider about modify from
  // https://github.com/llvm/llvm-project/blob/llvmorg-14.0.1/clang/tools/clang-scan-deps/ClangScanDeps.cpp
  //
  // TODO(zhangshuai.ustc): consider about using
  // https://github.com/llvm/llvm-project/blob/llvmorg-14.0.1/clang-tools-extra/clangd/CompileCommands.cpp

  return 0;
}
