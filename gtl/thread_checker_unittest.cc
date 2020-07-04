// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/thread_checker.h"

#include <functional>
#include <memory>
#include <thread>

#include "gtest/gtest.h"
#include "gtl/macros.h"
#include "gtl/sequence_token.h"

namespace gtl {
namespace {

// Runs a callback on a new thread synchronously.
void RunCallbackOnNewThreadSynchronously(std::function<void()> callback) {
  std::thread t(std::move(callback));
  t.join();
}

void ExpectCalledOnValidThread(ThreadCheckerImpl* thread_checker) {
  ASSERT_TRUE(thread_checker);

  // This should bind |thread_checker| to the current thread if it wasn't
  // already bound to a thread.
  EXPECT_TRUE(thread_checker->CalledOnValidThread());

  // Since |thread_checker| is now bound to the current thread, another call to
  // CalledOnValidThread() should return true.
  EXPECT_TRUE(thread_checker->CalledOnValidThread());
}

void ExpectNotCalledOnValidThread(ThreadCheckerImpl* thread_checker) {
  ASSERT_TRUE(thread_checker);
  EXPECT_FALSE(thread_checker->CalledOnValidThread());
}

void ExpectNotCalledOnValidThreadWithSequenceTokenAndThreadTaskRunnerHandle(
    ThreadCheckerImpl* thread_checker,
    SequenceToken sequence_token) {
  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(sequence_token);
  ExpectNotCalledOnValidThread(thread_checker);
}

}  // namespace

TEST(ThreadCheckerTest, AllowedSameThreadNoSequenceToken) {
  ThreadCheckerImpl thread_checker;
  EXPECT_TRUE(thread_checker.CalledOnValidThread());
}

TEST(ThreadCheckerTest,
     AllowedSameThreadAndSequenceDifferentTasksWithThreadTaskRunnerHandle) {
  std::unique_ptr<ThreadCheckerImpl> thread_checker;
  const SequenceToken sequence_token = SequenceToken::Create();

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(sequence_token);
    thread_checker.reset(new ThreadCheckerImpl);
  }

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(sequence_token);
    EXPECT_TRUE(thread_checker->CalledOnValidThread());
  }
}

TEST(ThreadCheckerTest,
     AllowedSameThreadSequenceAndTaskNoThreadTaskRunnerHandle) {
  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
  ThreadCheckerImpl thread_checker;
  EXPECT_TRUE(thread_checker.CalledOnValidThread());
}

TEST(ThreadCheckerTest,
     DisallowedSameThreadAndSequenceDifferentTasksNoThreadTaskRunnerHandle) {
  std::unique_ptr<ThreadCheckerImpl> thread_checker;

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    thread_checker.reset(new ThreadCheckerImpl);
  }

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    EXPECT_FALSE(thread_checker->CalledOnValidThread());
  }
}

TEST(ThreadCheckerTest, DisallowedDifferentThreadsNoSequenceToken) {
  ThreadCheckerImpl thread_checker;
  RunCallbackOnNewThreadSynchronously(
      [&thread_checker]() { ExpectNotCalledOnValidThread(&thread_checker); });
}

TEST(ThreadCheckerTest, DisallowedDifferentThreadsSameSequence) {
  const SequenceToken sequence_token(SequenceToken::Create());

  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(sequence_token);
  ThreadCheckerImpl thread_checker;
  EXPECT_TRUE(thread_checker.CalledOnValidThread());

  RunCallbackOnNewThreadSynchronously([&thread_checker, &sequence_token]() {
    ExpectNotCalledOnValidThreadWithSequenceTokenAndThreadTaskRunnerHandle(
        &thread_checker, sequence_token);
  });
}

TEST(ThreadCheckerTest, DisallowedSameThreadDifferentSequence) {
  std::unique_ptr<ThreadCheckerImpl> thread_checker;

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    thread_checker.reset(new ThreadCheckerImpl);
  }

  {
    // Different SequenceToken.
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    EXPECT_FALSE(thread_checker->CalledOnValidThread());
  }

  // No SequenceToken.
  EXPECT_FALSE(thread_checker->CalledOnValidThread());
}

TEST(ThreadCheckerTest, DetachFromThread) {
  ThreadCheckerImpl thread_checker;
  thread_checker.DetachFromThread();

  // Verify that CalledOnValidThread() returns true when called on a different
  // thread after a call to DetachFromThread().
  RunCallbackOnNewThreadSynchronously(
      [&thread_checker]() { ExpectCalledOnValidThread(&thread_checker); });

  EXPECT_FALSE(thread_checker.CalledOnValidThread());
}

TEST(ThreadCheckerTest, DetachFromThreadWithSequenceToken) {
  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
  ThreadCheckerImpl thread_checker;
  thread_checker.DetachFromThread();

  // Verify that CalledOnValidThread() returns true when called on a different
  // thread after a call to DetachFromThread().
  RunCallbackOnNewThreadSynchronously(
      [&thread_checker]() { ExpectCalledOnValidThread(&thread_checker); });

  EXPECT_FALSE(thread_checker.CalledOnValidThread());
}

// Owns a ThreadCheckerImpl and asserts that CalledOnValidThread() is valid
// in ~ThreadCheckerOwner.
class ThreadCheckerOwner {
 public:
  explicit ThreadCheckerOwner(bool detach_from_thread) {
    if (detach_from_thread)
      checker_.DetachFromThread();
  }
  ~ThreadCheckerOwner() { EXPECT_TRUE(checker_.CalledOnValidThread()); }

 private:
  ThreadCheckerImpl checker_;

  DISALLOW_COPY_AND_ASSIGN(ThreadCheckerOwner);
};

// Verifies ThreadCheckerImpl::CalledOnValidThread() returns true if called
// during thread destruction.
TEST(ThreadCheckerTest, CalledOnValidThreadFromThreadDestruction) {
  thread_local std::unique_ptr<ThreadCheckerOwner> thread_local_owner;
  RunCallbackOnNewThreadSynchronously([&]() {
    thread_local_owner = std::make_unique<ThreadCheckerOwner>(false);
  });
}

// Variant of CalledOnValidThreadFromThreadDestruction that calls
// ThreadCheckerImpl::DetachFromThread().
TEST(ThreadCheckerTest, CalledOnValidThreadFromThreadDestructionDetached) {
  thread_local std::unique_ptr<ThreadCheckerOwner> thread_local_owner;
  RunCallbackOnNewThreadSynchronously([&]() {
    thread_local_owner = std::make_unique<ThreadCheckerOwner>(true);
  });
}

TEST(ThreadCheckerTest, Move) {
  ThreadCheckerImpl initial;
  EXPECT_TRUE(initial.CalledOnValidThread());

  ThreadCheckerImpl move_constructed(std::move(initial));
  EXPECT_TRUE(move_constructed.CalledOnValidThread());

  ThreadCheckerImpl move_assigned;
  move_assigned = std::move(move_constructed);
  EXPECT_TRUE(move_assigned.CalledOnValidThread());

  // The two ThreadCheckerImpls moved from should be able to rebind to another
  // thread.
  RunCallbackOnNewThreadSynchronously(
      [&initial]() { ExpectCalledOnValidThread(&initial); });
  RunCallbackOnNewThreadSynchronously(
      [&move_constructed]() { ExpectCalledOnValidThread(&move_constructed); });

  // But the latest one shouldn't be able to run on another thread.
  RunCallbackOnNewThreadSynchronously(
      [&move_assigned]() { ExpectNotCalledOnValidThread(&move_assigned); });

  EXPECT_TRUE(move_assigned.CalledOnValidThread());
}

TEST(ThreadCheckerTest, MoveAssignIntoDetached) {
  ThreadCheckerImpl initial;

  ThreadCheckerImpl move_assigned;
  move_assigned.DetachFromThread();
  move_assigned = std::move(initial);

  // |initial| is detached after move.
  RunCallbackOnNewThreadSynchronously(
      [&initial]() { ExpectCalledOnValidThread(&initial); });

  // |move_assigned| should be associated with the main thread.
  RunCallbackOnNewThreadSynchronously(
      [&move_assigned]() { ExpectNotCalledOnValidThread(&move_assigned); });

  EXPECT_TRUE(move_assigned.CalledOnValidThread());
}

TEST(ThreadCheckerTest, MoveFromDetachedRebinds) {
  ThreadCheckerImpl initial;
  initial.DetachFromThread();

  ThreadCheckerImpl moved_into(std::move(initial));

  // |initial| is still detached after move.
  RunCallbackOnNewThreadSynchronously(
      [&initial]() { ExpectCalledOnValidThread(&initial); });

  // |moved_into| is bound to the current thread as part of the move.
  RunCallbackOnNewThreadSynchronously(
      [&moved_into]() { ExpectNotCalledOnValidThread(&moved_into); });
  EXPECT_TRUE(moved_into.CalledOnValidThread());
}

// TEST(ThreadCheckerTest, MoveOffThreadBanned) {
//   testing::GTEST_FLAG(death_test_style) = "threadsafe";

//   ThreadCheckerImpl other_thread;
//   other_thread.DetachFromThread();
//   RunCallbackOnNewThreadSynchronously(
//       [&other_thread]() { ExpectCalledOnValidThread(&other_thread); });

//   EXPECT_DCHECK_DEATH(ThreadCheckerImpl main_thread(std::move(other_thread)));
// }

// namespace {

// // This fixture is a helper for unit testing the thread checker macros as it is
// // not possible to inline ExpectDeathOnOtherThread() and
// // ExpectNoDeathOnOtherThreadAfterDetach() as lambdas since binding
// // |Unretained(&my_sequence_checker)| wouldn't compile on non-dcheck builds
// // where it won't be defined.
// class ThreadCheckerMacroTest : public testing::Test {
//  public:
//   ThreadCheckerMacroTest() = default;

//   void ExpectDeathOnOtherThread() {
// #if DCHECK_IS_ON()
//     EXPECT_DCHECK_DEATH({ DCHECK_CALLED_ON_VALID_THREAD(thread_checker_); });
// #else
//     // Happily no-ops on non-dcheck builds.
//     DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
// #endif
//   }

//   void ExpectNoDeathOnOtherThreadAfterDetach() {
//     DCHECK_CALLED_ON_VALID_THREAD(thread_checker_);
//   }

//  protected:
//   THREAD_CHECKER(thread_checker_);

//  private:
//   DISALLOW_COPY_AND_ASSIGN(ThreadCheckerMacroTest);
// };

// }  // namespace

// TEST_F(ThreadCheckerMacroTest, Macros) {
//   testing::GTEST_FLAG(death_test_style) = "threadsafe";

//   THREAD_CHECKER(my_thread_checker);

//   RunCallbackOnNewThreadSynchronously(BindOnce(
//       &ThreadCheckerMacroTest::ExpectDeathOnOtherThread, Unretained(this)));

//   DETACH_FROM_THREAD(thread_checker_);

//   RunCallbackOnNewThreadSynchronously(
//       BindOnce(&ThreadCheckerMacroTest::ExpectNoDeathOnOtherThreadAfterDetach,
//                Unretained(this)));
// }

}  // namespace gtl
