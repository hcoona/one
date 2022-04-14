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
// https://github.com/tensorflow/tensorflow/blob/v2.8.0/tensorflow/core/platform/default/posix_file_system.h
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
// 3. Use `uint*_t` instead of `uint*`, `std::string_view` instead of
//    `StringPiece`.
// 4. Remove `TranslateName`.
// 5. Only keep `NewReadOnlyMemoryRegionFromFile` for now. Would add back other
//    methods later.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "one/base/filesystem/file_system.h"

namespace hcoona {

class PosixFileSystem : public FileSystem {
 public:
  absl::Status NewReadOnlyMemoryRegionFromFile(
      const std::string& filename,
      std::unique_ptr<ReadOnlyMemoryRegion>* result) override;
};

}  // namespace hcoona
