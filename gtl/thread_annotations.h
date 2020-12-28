// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This header file contains macro definitions for thread safety annotations
// that allow developers to document the locking policies of multi-threaded
// code. The annotations can also help program analysis tools to identify
// potential thread safety issues.
//
// Note that the annotations we use are described as deprecated in the Clang
// documentation, linked below. E.g. we use ABSL_EXCLUSIVE_LOCKS_REQUIRED where
// the Clang docs use REQUIRES.
//
// http://clang.llvm.org/docs/ThreadSafetyAnalysis.html
//
// We use the deprecated Clang annotations to match Abseil (relevant header
// linked below) and its ecosystem of libraries. We will follow Abseil with
// respect to upgrading to more modern annotations.
//
// https://github.com/abseil/abseil-cpp/blob/master/absl/base/thread_annotations.h
//
// These annotations are implemented using compiler attributes. Using the macros
// defined here instead of raw attributes allow for portability and future
// compatibility.
//
// When referring to mutexes in the arguments of the attributes, you should
// use variable names or more complex expressions (e.g. my_object->mutex_)
// that evaluate to a concrete mutex object whenever possible. If the mutex
// you want to refer to is not in scope, you may use a member pointer
// (e.g. &MyClass::mutex_) to refer to a mutex in some (unknown) object.

#ifndef GTL_THREAD_ANNOTATIONS_H_
#define GTL_THREAD_ANNOTATIONS_H_

#include "absl/base/thread_annotations.h"
#include "glog/logging.h"

// The above is imported as-is from abseil-cpp. The following Chromium-specific
// synonyms are added for Chromium concepts (SequenceChecker/ThreadChecker).
#if DCHECK_IS_ON()

// Equivalent to ABSL_GUARDED_BY for SequenceChecker/ThreadChecker. Currently,
// clang's error message "requires holding mutex" is misleading. Usage of this
// macro is discouraged until the message is updated.
// TODO(etiennep): Update comment above once clang's error message is updated.
#define GTL_GUARDED_BY_CONTEXT(name) ABSL_GUARDED_BY(name)

// Equivalent to ABSL_EXCLUSIVE_LOCKS_REQUIRED for
// SequenceChecker/ThreadChecker. Currently, clang's error message "requires
// holding mutex" is misleading. Usage of this macro is discouraged until the
// message is updated.
// TODO(etiennep): Update comment above once clang's error message is updated.
#define GTL_VALID_CONTEXT_REQUIRED(name) ABSL_EXCLUSIVE_LOCKS_REQUIRED(name)

#else  // DCHECK_IS_ON()

#define GTL_GUARDED_BY_CONTEXT(name)
#define GTL_VALID_CONTEXT_REQUIRED(name)

#endif  // DCHECK_IS_ON()

#endif  // GTL_THREAD_ANNOTATIONS_H_
