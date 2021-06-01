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

#include "gandiva/cache.h"
#include "third_party/arrow/src/arrow/util/logging.h"

namespace gandiva {

static const int DEFAULT_CACHE_SIZE = 500;

int GetCapacity() {
  int capacity;
  const char* env_cache_size = std::getenv("GANDIVA_CACHE_SIZE");
  if (env_cache_size != nullptr) {
    capacity = std::atoi(env_cache_size);
    if (capacity <= 0) {
      ARROW_LOG(WARNING) << "Invalid cache size provided. Using default cache size: "
                         << DEFAULT_CACHE_SIZE;
      capacity = DEFAULT_CACHE_SIZE;
    }
  } else {
    capacity = DEFAULT_CACHE_SIZE;
  }
  return capacity;
}

void LogCacheSize(size_t capacity) {
  ARROW_LOG(INFO) << "Creating gandiva cache with capacity: " << capacity;
}

}  // namespace gandiva
