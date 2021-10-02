/* Copyright 2015 The TensorFlow Authors. All Rights Reserved.

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

#ifndef GTL_POSIX_FILE_SYSTEM_H_
#define GTL_POSIX_FILE_SYSTEM_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "gtl/file_system.h"
#include "gtl/path.h"

namespace gtl {

class PosixFileSystem : public FileSystem {
 public:
  PosixFileSystem() = default;

  ~PosixFileSystem() override = default;

  absl::Status NewRandomAccessFile(
      const std::string& filename,
      std::unique_ptr<RandomAccessFile>* result) override;

  absl::Status NewWritableFile(const std::string& fname,
                         std::unique_ptr<WritableFile>* result) override;

  absl::Status NewAppendableFile(const std::string& fname,
                           std::unique_ptr<WritableFile>* result) override;

  absl::Status NewReadOnlyMemoryRegionFromFile(
      const std::string& filename,
      std::unique_ptr<ReadOnlyMemoryRegion>* result) override;

  absl::Status FileExists(const std::string& fname) override;

  absl::Status GetChildren(const std::string& dir, std::vector<std::string>* result) override;

  absl::Status Stat(const std::string& fname, FileStatistics* stats) override;

  absl::Status GetMatchingPaths(const std::string& pattern,
                          std::vector<std::string>* results) override;

  absl::Status DeleteFile(const std::string& fname) override;

  absl::Status CreateDir(const std::string& name) override;

  absl::Status DeleteDir(const std::string& name) override;

  absl::Status GetFileSize(const std::string& fname, uint64_t* size) override;

  absl::Status RenameFile(const std::string& src, const std::string& target) override;

  absl::Status CopyFile(const std::string& src, const std::string& target) override;
};

class LocalPosixFileSystem : public PosixFileSystem {
 public:
  std::string TranslateName(const std::string& name) const override {
    absl::string_view scheme, host, path;
    ParseURI(name, &scheme, &host, &path);
    return std::string(path);
  }
};

absl::Status IOError(const std::string& context, int err_number);

}  // namespace gtl

#endif  // GTL_POSIX_FILE_SYSTEM_H_
