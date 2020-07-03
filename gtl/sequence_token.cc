// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/sequence_token.h"

#include "gtl/atomic_sequence_num.h"
#include "glog/logging.h"

namespace gtl {

namespace {

gtl::AtomicSequenceNumber g_sequence_token_generator;

gtl::AtomicSequenceNumber g_task_token_generator;

const SequenceToken** GetTlsCurrentSequenceToken() {
  static thread_local const SequenceToken* instance;
  return &instance;
}

const TaskToken** GetTlsCurrentTaskToken() {
  static thread_local const TaskToken* instance;
  return &instance;
}

}  // namespace

bool SequenceToken::operator==(const SequenceToken& other) const {
  return token_ == other.token_ && IsValid();
}

bool SequenceToken::operator!=(const SequenceToken& other) const {
  return !(*this == other);
}

bool SequenceToken::IsValid() const {
  return token_ != kInvalidSequenceToken;
}

int SequenceToken::ToInternalValue() const {
  return token_;
}

SequenceToken SequenceToken::Create() {
  return SequenceToken(g_sequence_token_generator.GetNext());
}

SequenceToken SequenceToken::GetForCurrentThread() {
  const SequenceToken* current_sequence_token =
      *(GetTlsCurrentSequenceToken());
  return current_sequence_token ? *current_sequence_token : SequenceToken();
}

bool TaskToken::operator==(const TaskToken& other) const {
  return token_ == other.token_ && IsValid();
}

bool TaskToken::operator!=(const TaskToken& other) const {
  return !(*this == other);
}

bool TaskToken::IsValid() const {
  return token_ != kInvalidTaskToken;
}

TaskToken TaskToken::Create() {
  return TaskToken(g_task_token_generator.GetNext());
}

TaskToken TaskToken::GetForCurrentThread() {
  const TaskToken* current_task_token = *(GetTlsCurrentTaskToken());
  return current_task_token ? *current_task_token : TaskToken();
}

ScopedSetSequenceTokenForCurrentThread::ScopedSetSequenceTokenForCurrentThread(
    const SequenceToken& sequence_token)
    : sequence_token_(sequence_token), task_token_(TaskToken::Create()) {
  DCHECK(!*(GetTlsCurrentSequenceToken()));
  DCHECK(!*(GetTlsCurrentTaskToken()));
  *(GetTlsCurrentSequenceToken()) = &sequence_token_;
  *(GetTlsCurrentTaskToken()) = &task_token_;
}

ScopedSetSequenceTokenForCurrentThread::
    ~ScopedSetSequenceTokenForCurrentThread() {
  DCHECK_EQ(*(GetTlsCurrentSequenceToken()), &sequence_token_);
  DCHECK_EQ(*(GetTlsCurrentTaskToken()), &task_token_);
  *(GetTlsCurrentSequenceToken()) = nullptr;
  *(GetTlsCurrentTaskToken()) = nullptr;
}

}  // namespace gtl
