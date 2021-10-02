// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GTL_THREAD_CHECKER_IMPL_H_
#define GTL_THREAD_CHECKER_IMPL_H_

#include <thread>

#include "absl/synchronization/mutex.h"
#include "absl/types/optional.h"
#include "gtl/compiler_specific.h"
#include "gtl/sequence_token.h"
#include "gtl/thread_annotations.h"

namespace gtl {

// Real implementation of ThreadChecker, for use in debug mode, or for temporary
// use in release mode (e.g. to CHECK on a threading issue seen only in the
// wild).
//
// Note: You should almost always use the ThreadChecker class to get the right
// version for your build configuration.
// Note: This is only a check, not a "lock". It is marked "LOCKABLE" only in
// order to support thread_annotations.h.
class LOCKABLE ThreadCheckerImpl {
 public:
  ThreadCheckerImpl();
  ~ThreadCheckerImpl();

  // Allow move construct/assign. This must be called on |other|'s associated
  // thread and assignment can only be made into a ThreadCheckerImpl which is
  // detached or already associated with the current thread. This isn't
  // thread-safe (|this| and |other| shouldn't be in use while this move is
  // performed). If the assignment was legal, the resulting ThreadCheckerImpl
  // will be bound to the current thread and |other| will be detached.
  ThreadCheckerImpl(ThreadCheckerImpl&& other);
  ThreadCheckerImpl& operator=(ThreadCheckerImpl&& other);

  bool CalledOnValidThread() const WARN_UNUSED_RESULT;

  // Changes the thread that is checked for in CalledOnValidThread.  This may
  // be useful when an object may be created on one thread and then used
  // exclusively on another thread.
  void DetachFromThread();

 private:
  void EnsureAssignedLockRequired() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(lock_);

  // Members are mutable so that CalledOnValidThread() can set them.

  // Synchronizes access to all members.
  mutable absl::Mutex lock_;

  // Thread on which CalledOnValidThread() may return true.
  mutable absl::optional<std::thread::id> thread_id_ ABSL_GUARDED_BY(lock_);

  // TaskToken for which CalledOnValidThread() always returns true. This allows
  // CalledOnValidThread() to return true when called multiple times from the
  // same task, even if it's not running in a single-threaded context itself
  // (allowing usage of ThreadChecker objects on the stack in the scope of one-
  // off tasks). Note: CalledOnValidThread() may return true even if the current
  // TaskToken is not equal to this.
  mutable TaskToken task_token_ ABSL_GUARDED_BY(lock_);

  // SequenceToken for which CalledOnValidThread() may return true. Used to
  // ensure that CalledOnValidThread() doesn't return true for ThreadPool
  // tasks that happen to run on the same thread but weren't posted to the same
  // SingleThreadTaskRunner.
  mutable SequenceToken sequence_token_ ABSL_GUARDED_BY(lock_);
};

}  // namespace gtl

#endif  // GTL_THREAD_CHECKER_IMPL_H_
