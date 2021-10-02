/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef GTL_NULL_FILE_SYSTEM_H_
#define GTL_NULL_FILE_SYSTEM_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "gtl/file_statistics.h"
#include "gtl/file_system.h"
#include "gtl/file_system_helper.h"

namespace gtl {

// START_SKIP_DOXYGEN

#ifndef SWIG
// Degenerate file system that provides no implementations.
class NullFileSystem : public FileSystem {
 public:
  NullFileSystem() = default;

  ~NullFileSystem() override = default;

  absl::Status NewRandomAccessFile(
      const std::string& fname, std::unique_ptr<RandomAccessFile>* result) override {
    return absl::UnimplementedError("NewRandomAccessFile unimplemented");
  }

  absl::Status NewWritableFile(const std::string& fname,
                         std::unique_ptr<WritableFile>* result) override {
    return absl::UnimplementedError("NewWritableFile unimplemented");
  }

  absl::Status NewAppendableFile(const std::string& fname,
                           std::unique_ptr<WritableFile>* result) override {
    return absl::UnimplementedError("NewAppendableFile unimplemented");
  }

  absl::Status NewReadOnlyMemoryRegionFromFile(
      const std::string& fname,
      std::unique_ptr<ReadOnlyMemoryRegion>* result) override {
    return absl::UnimplementedError(
        "NewReadOnlyMemoryRegionFromFile unimplemented");
  }

  absl::Status FileExists(const std::string& fname) override {
    return absl::UnimplementedError("FileExists unimplemented");
  }

  absl::Status GetChildren(const std::string& dir, std::vector<std::string>* result) override {
    return absl::UnimplementedError("GetChildren unimplemented");
  }

  absl::Status GetMatchingPaths(const std::string& pattern,
                          std::vector<std::string>* results) override {
    return internal::GetMatchingPaths(this, pattern, results);
  }

  absl::Status DeleteFile(const std::string& fname) override {
    return absl::UnimplementedError("DeleteFile unimplemented");
  }

  absl::Status CreateDir(const std::string& dirname) override {
    return absl::UnimplementedError("CreateDir unimplemented");
  }

  absl::Status DeleteDir(const std::string& dirname) override {
    return absl::UnimplementedError("DeleteDir unimplemented");
  }

  absl::Status GetFileSize(const std::string& fname, uint64_t* file_size) override {
    return absl::UnimplementedError("GetFileSize unimplemented");
  }

  absl::Status RenameFile(const std::string& src, const std::string& target) override {
    return absl::UnimplementedError("RenameFile unimplemented");
  }

  absl::Status Stat(const std::string& fname, FileStatistics* stat) override {
    return absl::UnimplementedError("Stat unimplemented");
  }
};
#endif

// END_SKIP_DOXYGEN

}  // namespace gtl

#endif  // GTL_NULL_FILE_SYSTEM_H_
