// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/sequence_checker.h"

#include <stddef.h>

#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "gtest/gtest.h"
#include "gtl/macros.h"
#include "gtl/sequence_token.h"
#include "gtl/test/gtest_util.h"

namespace gtl {

namespace {

// Runs a callback on another thread.
class RunCallbackThread : public std::thread {
 public:
  explicit RunCallbackThread(std::function<void()> callback)
      : std::thread(std::move(callback)) {
    join();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(RunCallbackThread);
};

void ExpectCalledOnValidSequence(SequenceCheckerImpl* sequence_checker) {
  ASSERT_TRUE(sequence_checker);

  // This should bind |sequence_checker| to the current sequence if it wasn't
  // already bound to a sequence.
  EXPECT_TRUE(sequence_checker->CalledOnValidSequence());

  // Since |sequence_checker| is now bound to the current sequence, another call
  // to CalledOnValidSequence() should return true.
  EXPECT_TRUE(sequence_checker->CalledOnValidSequence());
}

void ExpectCalledOnValidSequenceWithSequenceToken(
    SequenceCheckerImpl* sequence_checker, SequenceToken sequence_token) {
  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(sequence_token);
  ExpectCalledOnValidSequence(sequence_checker);
}

void ExpectNotCalledOnValidSequence(SequenceCheckerImpl* sequence_checker) {
  ASSERT_TRUE(sequence_checker);
  EXPECT_FALSE(sequence_checker->CalledOnValidSequence());
}

}  // namespace

TEST(SequenceCheckerTest, CallsAllowedOnSameThreadNoSequenceToken) {
  SequenceCheckerImpl sequence_checker;
  EXPECT_TRUE(sequence_checker.CalledOnValidSequence());
}

TEST(SequenceCheckerTest, CallsAllowedOnSameThreadSameSequenceToken) {
  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
  SequenceCheckerImpl sequence_checker;
  EXPECT_TRUE(sequence_checker.CalledOnValidSequence());
}

TEST(SequenceCheckerTest, CallsDisallowedOnDifferentThreadsNoSequenceToken) {
  SequenceCheckerImpl sequence_checker;
  RunCallbackThread thread([&sequence_checker]() {
    ExpectNotCalledOnValidSequence(&sequence_checker);
  });
}

TEST(SequenceCheckerTest, CallsAllowedOnDifferentThreadsSameSequenceToken) {
  const SequenceToken sequence_token(SequenceToken::Create());

  ScopedSetSequenceTokenForCurrentThread
      scoped_set_sequence_token_for_current_thread(sequence_token);
  SequenceCheckerImpl sequence_checker;
  EXPECT_TRUE(sequence_checker.CalledOnValidSequence());

  RunCallbackThread thread([&sequence_checker, sequence_token]() {
    ExpectCalledOnValidSequenceWithSequenceToken(&sequence_checker,
                                                 sequence_token);
  });
}

TEST(SequenceCheckerTest, CallsDisallowedOnSameThreadDifferentSequenceToken) {
  std::unique_ptr<SequenceCheckerImpl> sequence_checker;

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    sequence_checker.reset(new SequenceCheckerImpl);
  }

  {
    // Different SequenceToken.
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    EXPECT_FALSE(sequence_checker->CalledOnValidSequence());
  }

  // No SequenceToken.
  EXPECT_FALSE(sequence_checker->CalledOnValidSequence());
}

TEST(SequenceCheckerTest, DetachFromSequence) {
  std::unique_ptr<SequenceCheckerImpl> sequence_checker;

  {
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    sequence_checker.reset(new SequenceCheckerImpl);
  }

  sequence_checker->DetachFromSequence();

  {
    // Verify that CalledOnValidSequence() returns true when called with
    // a different sequence token after a call to DetachFromSequence().
    ScopedSetSequenceTokenForCurrentThread
        scoped_set_sequence_token_for_current_thread(SequenceToken::Create());
    EXPECT_TRUE(sequence_checker->CalledOnValidSequence());
  }
}

TEST(SequenceCheckerTest, DetachFromSequenceNoSequenceToken) {
  SequenceCheckerImpl sequence_checker;
  sequence_checker.DetachFromSequence();

  // Verify that CalledOnValidSequence() returns true when called on a
  // different thread after a call to DetachFromSequence().
  RunCallbackThread thread([&sequence_checker]() {
    ExpectCalledOnValidSequence(&sequence_checker);
  });

  EXPECT_FALSE(sequence_checker.CalledOnValidSequence());
}

TEST(SequenceCheckerTest, Move) {
  SequenceCheckerImpl initial;
  EXPECT_TRUE(initial.CalledOnValidSequence());

  SequenceCheckerImpl move_constructed(std::move(initial));
  EXPECT_TRUE(move_constructed.CalledOnValidSequence());

  SequenceCheckerImpl move_assigned;
  move_assigned = std::move(move_constructed);

  // The two SequenceCheckerImpls moved from should be able to rebind to another
  // sequence.
  RunCallbackThread thread1(
      [&initial]() { ExpectCalledOnValidSequence(&initial); });
  RunCallbackThread thread2([&move_constructed]() {
    ExpectCalledOnValidSequence(&move_constructed);
  });

  // But the latest one shouldn't be able to run on another sequence.
  RunCallbackThread thread(
      [&move_assigned]() { ExpectNotCalledOnValidSequence(&move_assigned); });

  EXPECT_TRUE(move_assigned.CalledOnValidSequence());
}

TEST(SequenceCheckerTest, MoveAssignIntoDetached) {
  SequenceCheckerImpl initial;

  SequenceCheckerImpl move_assigned;
  move_assigned.DetachFromSequence();
  move_assigned = std::move(initial);

  // |initial| is detached after move.
  RunCallbackThread thread1(
      [&initial]() { ExpectCalledOnValidSequence(&initial); });

  // |move_assigned| should be associated with the main thread.
  RunCallbackThread thread2(
      [&move_assigned]() { ExpectNotCalledOnValidSequence(&move_assigned); });

  EXPECT_TRUE(move_assigned.CalledOnValidSequence());
}

TEST(SequenceCheckerTest, MoveFromDetachedRebinds) {
  SequenceCheckerImpl initial;
  initial.DetachFromSequence();

  SequenceCheckerImpl moved_into(std::move(initial));

  // |initial| is still detached after move.
  RunCallbackThread thread1(
      [&initial]() { ExpectCalledOnValidSequence(&initial); });

  // |moved_into| is bound to the current sequence as part of the move.
  RunCallbackThread thread2(
      [&moved_into]() { ExpectNotCalledOnValidSequence(&moved_into); });
  EXPECT_TRUE(moved_into.CalledOnValidSequence());
}

TEST(SequenceCheckerTest, MoveOffSequenceBanned) {
  testing::GTEST_FLAG(death_test_style) = "threadsafe";

  SequenceCheckerImpl other_sequence;
  other_sequence.DetachFromSequence();
  RunCallbackThread thread(
      [&other_sequence]() { ExpectCalledOnValidSequence(&other_sequence); });

  EXPECT_DCHECK_DEATH(
      SequenceCheckerImpl main_sequence(std::move(other_sequence)));
}

TEST(SequenceCheckerMacroTest, Macros) {
  auto scope = std::make_unique<ScopedSetSequenceTokenForCurrentThread>(
      SequenceToken::Create());
  SEQUENCE_CHECKER(my_sequence_checker);

  {
    // Don't expect a DCHECK death when a SequenceChecker is used on the right
    // sequence.
    DCHECK_CALLED_ON_VALID_SEQUENCE(my_sequence_checker);
  }
  scope.reset();

#if DCHECK_IS_ON()
  // Expect DCHECK death when used on a different sequence.
  EXPECT_DCHECK_DEATH(
      { DCHECK_CALLED_ON_VALID_SEQUENCE(my_sequence_checker); });
#else
  // Happily no-ops on non-dcheck builds.
  DCHECK_CALLED_ON_VALID_SEQUENCE(my_sequence_checker);
#endif

  DETACH_FROM_SEQUENCE(my_sequence_checker);

  // Don't expect a DCHECK death when a SequenceChecker is used for the first
  // time after having been detached.
  DCHECK_CALLED_ON_VALID_SEQUENCE(my_sequence_checker);
}

// Owns a SequenceCheckerImpl, and asserts that CalledOnValidSequence() is valid
// in ~SequenceCheckerOwner.
class SequenceCheckerOwner {
 public:
  SequenceCheckerOwner() = default;
  ~SequenceCheckerOwner() { EXPECT_TRUE(checker_.CalledOnValidSequence()); }

 private:
  SequenceCheckerImpl checker_;

  DISALLOW_COPY_AND_ASSIGN(SequenceCheckerOwner);
};

// // Verifies SequenceCheckerImpl::CalledOnValidSequence() returns true if
// called
// // during thread destruction.
// TEST(SequenceCheckerTest, CalledOnValidSequenceFromThreadDestruction) {
//   ThreadLocalOwnedPointer<SequenceCheckerOwner> thread_local_owner;
//   {
//     test::TaskEnvironment task_environment;
//     auto task_runner = ThreadPool::CreateSequencedTaskRunner({});
//     task_runner->PostTask(
//         FROM_HERE, BindLambdaForTesting([&]() {
//           thread_local_owner.Set(std::make_unique<SequenceCheckerOwner>());
//         }));
//     task_runner = nullptr;
//     task_environment.RunUntilIdle();
//   }
// }

}  // namespace gtl
