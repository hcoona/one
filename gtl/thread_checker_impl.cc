// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/thread_checker_impl.h"

#include "glog/logging.h"

namespace gtl {

ThreadCheckerImpl::ThreadCheckerImpl() {
  absl::MutexLock mutex_lock(&lock_);
  EnsureAssignedLockRequired();
}

ThreadCheckerImpl::~ThreadCheckerImpl() = default;

ThreadCheckerImpl::ThreadCheckerImpl(ThreadCheckerImpl&& other) {
  // Verify that |other| is called on its associated thread and bind it now if
  // it is currently detached (even if this isn't a DCHECK build).
  const bool other_called_on_valid_thread = other.CalledOnValidThread();
  DCHECK(other_called_on_valid_thread);

  // Intentionally not using |other.lock_| to let TSAN catch racy construct from
  // |other|.
  thread_id_ = other.thread_id_;
  task_token_ = other.task_token_;
  sequence_token_ = other.sequence_token_;

  other.thread_id_.reset();
  other.task_token_ = TaskToken();
  other.sequence_token_ = SequenceToken();
}

ThreadCheckerImpl& ThreadCheckerImpl::operator=(ThreadCheckerImpl&& other) {
  DCHECK(CalledOnValidThread());

  // Verify that |other| is called on its associated thread and bind it now if
  // it is currently detached (even if this isn't a DCHECK build).
  const bool other_called_on_valid_thread = other.CalledOnValidThread();
  DCHECK(other_called_on_valid_thread);

  // Intentionally not using either |lock_| to let TSAN catch racy assign.
  TS_UNCHECKED_READ(thread_id_) = TS_UNCHECKED_READ(other.thread_id_);
  TS_UNCHECKED_READ(task_token_) = TS_UNCHECKED_READ(other.task_token_);
  TS_UNCHECKED_READ(sequence_token_) = TS_UNCHECKED_READ(other.sequence_token_);

  TS_UNCHECKED_READ(other.thread_id_).reset();
  TS_UNCHECKED_READ(other.task_token_) = TaskToken();
  TS_UNCHECKED_READ(other.sequence_token_) = SequenceToken();

  return *this;
}

bool ThreadCheckerImpl::CalledOnValidThread() const {
  const bool has_thread_been_destroyed = false;

  absl::MutexLock mutex_lock(&lock_);
  // TaskToken/SequenceToken access thread-local storage. During destruction
  // the state of thread-local storage is not guaranteed to be in a consistent
  // state. Further, task-runner only installs the tokens when running a task.
  if (!has_thread_been_destroyed) {
    EnsureAssignedLockRequired();

    // Always return true when called from the task from which this
    // ThreadCheckerImpl was assigned to a thread.
    if (task_token_ == TaskToken::GetForCurrentThread())
      return true;

    // If this ThreadCheckerImpl is bound to a valid SequenceToken, it must be
    // equal to the current SequenceToken and there must be a registered
    // ThreadTaskRunnerHandle. Otherwise, the fact that the current task runs on
    // the thread to which this ThreadCheckerImpl is bound is fortuitous.
    if (sequence_token_.IsValid() &&
        (sequence_token_ != SequenceToken::GetForCurrentThread())) {
      return false;
    }
  }

  return thread_id_ == std::this_thread::get_id();
}

void ThreadCheckerImpl::DetachFromThread() {
  absl::MutexLock mutex_lock(&lock_);
  task_token_ = TaskToken();
  sequence_token_ = SequenceToken();
}

void ThreadCheckerImpl::EnsureAssignedLockRequired() const {
  if (thread_id_.has_value())
    return;

  thread_id_ = std::this_thread::get_id();
  task_token_ = TaskToken::GetForCurrentThread();
  sequence_token_ = SequenceToken::GetForCurrentThread();
}

}  // namespace gtl
