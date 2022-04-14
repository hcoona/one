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
//
// Copied from
// https://github.com/tensorflow/tensorflow/blob/v2.8.0/tensorflow/core/platform/file_system.h
//
// Copyright 2015 The TensorFlow Authors. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Modifications:
// 1. Fix clang-tidy complains.
// 2. Use `absl::Status` instead of `tensorflow::Status`.
// 3. Use `uint*_t` instead of `uint*`.
// 4. Remove `TranslateName`.
// 5. Only keep `NewReadOnlyMemoryRegionFromFile` for now. Would add back other
//    methods later.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "absl/status/status.h"

namespace hcoona {

/// \brief A readonly memmapped file abstraction.
///
/// The implementation must guarantee that all memory is accessible when the
/// object exists, independently from the Env that created it.
class ReadOnlyMemoryRegion {
 public:
  ReadOnlyMemoryRegion() = default;
  virtual ~ReadOnlyMemoryRegion() = default;

  ReadOnlyMemoryRegion(const ReadOnlyMemoryRegion&) = delete;
  ReadOnlyMemoryRegion& operator=(const ReadOnlyMemoryRegion&) = delete;

  ReadOnlyMemoryRegion(ReadOnlyMemoryRegion&&) = default;
  ReadOnlyMemoryRegion& operator=(ReadOnlyMemoryRegion&&) = default;

  /// \brief Returns a pointer to the memory region.
  virtual const void* data() = 0;

  /// \brief Returns the length of the memory region in bytes.
  virtual uint64_t length() = 0;
};

/// A generic interface for accessing a file system.  Implementations
/// of custom filesystem adapters must implement this interface,
/// RandomAccessFile, WritableFile, and ReadOnlyMemoryRegion classes.
class FileSystem {
 public:
  FileSystem() = default;
  virtual ~FileSystem() = default;

  FileSystem(const FileSystem&) = delete;
  FileSystem& operator=(const FileSystem&) = delete;

  FileSystem(FileSystem&&) = default;
  FileSystem& operator=(FileSystem&&) = delete;

  /// \brief Creates a readonly region of memory with the file context.
  ///
  /// On success, it returns a pointer to read-only memory region
  /// from the content of file fname. The ownership of the region is passed to
  /// the caller. On failure stores nullptr in *result and returns non-OK.
  ///
  /// The returned memory region can be accessed from many threads in parallel.
  ///
  /// The ownership of the returned ReadOnlyMemoryRegion is passed to the caller
  /// and the object should be deleted when is not used.
  virtual absl::Status NewReadOnlyMemoryRegionFromFile(
      const std::string& /*fname*/,
      std::unique_ptr<ReadOnlyMemoryRegion>* /*result*/) {
    return absl::UnimplementedError("Not implemented yet.");
  }
};

}  // namespace hcoona
