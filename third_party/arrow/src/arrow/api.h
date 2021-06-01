// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// Coarse public API while the library is in development

#pragma once

#include "third_party/arrow/src/arrow/array.h"                    // IYWU pragma: export
#include "third_party/arrow/src/arrow/array/concatenate.h"        // IYWU pragma: export
#include "third_party/arrow/src/arrow/buffer.h"                   // IYWU pragma: export
#include "third_party/arrow/src/arrow/builder.h"                  // IYWU pragma: export
#include "third_party/arrow/src/arrow/chunked_array.h"            // IYWU pragma: export
#include "third_party/arrow/src/arrow/compare.h"                  // IYWU pragma: export
#include "third_party/arrow/src/arrow/config.h"                   // IYWU pragma: export
#include "third_party/arrow/src/arrow/datum.h"                    // IYWU pragma: export
#include "third_party/arrow/src/arrow/extension_type.h"           // IYWU pragma: export
#include "third_party/arrow/src/arrow/memory_pool.h"              // IYWU pragma: export
#include "third_party/arrow/src/arrow/pretty_print.h"             // IYWU pragma: export
#include "third_party/arrow/src/arrow/record_batch.h"             // IYWU pragma: export
#include "third_party/arrow/src/arrow/result.h"                   // IYWU pragma: export
#include "third_party/arrow/src/arrow/status.h"                   // IYWU pragma: export
#include "third_party/arrow/src/arrow/table.h"                    // IYWU pragma: export
#include "third_party/arrow/src/arrow/table_builder.h"            // IYWU pragma: export
#include "third_party/arrow/src/arrow/tensor.h"                   // IYWU pragma: export
#include "third_party/arrow/src/arrow/type.h"                     // IYWU pragma: export
#include "third_party/arrow/src/arrow/util/key_value_metadata.h"  // IWYU pragma: export
#include "third_party/arrow/src/arrow/visitor.h"                  // IYWU pragma: export

/// \brief Top-level namespace for Apache Arrow C++ API
namespace arrow {}
