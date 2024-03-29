/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "folly/fibers/EventBaseLoopController.h"
#include "folly/fibers/FiberManagerInternal.h"
#include "folly/io/async/VirtualEventBase.h"

namespace folly {
namespace fibers {

template <typename Local>
static inline FiberManager& getFiberManagerT(
    folly::EventBase& evb,
    const FiberManager::Options& opts = FiberManager::Options());

static inline FiberManager& getFiberManager(
    folly::EventBase& evb,
    const FiberManager::Options& opts = FiberManager::Options());

static inline FiberManager& getFiberManager(
    folly::VirtualEventBase& evb,
    const FiberManager::Options& opts = FiberManager::Options());

static inline FiberManager& getFiberManager(
    folly::EventBase& evb, const FiberManager::FrozenOptions& opts);

static inline FiberManager& getFiberManager(
    folly::VirtualEventBase& evb, const FiberManager::FrozenOptions& opts);
} // namespace fibers
} // namespace folly
#include "folly/fibers/FiberManagerMap-inl.h"
