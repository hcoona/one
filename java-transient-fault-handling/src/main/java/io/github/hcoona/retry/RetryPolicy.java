package io.github.hcoona.retry;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Preconditions;
import java.time.Duration;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ScheduledExecutorService;

/**
 * Provides the base implementation of the retry mechanism for unreliable actions and transient
 * conditions.
 */
public class RetryPolicy {
  private static RetryPolicy noRetry;
  private static RetryPolicy defaultFixed;
  private static RetryPolicy defaultProgressive;
  private static RetryPolicy defaultExponential;

  static {
    try {
      noRetry = new RetryPolicy(new TransientErrorIgnoreStrategy(), RetryStrategy.getNoRetry());
      defaultFixed =
          new RetryPolicy(new TransientErrorCatchAllStrategy(), RetryStrategy.getDefaultFixed());
      defaultProgressive =
          new RetryPolicy(
              new TransientErrorCatchAllStrategy(), RetryStrategy.getDefaultProgressive());
      defaultExponential =
          new RetryPolicy(
              new TransientErrorCatchAllStrategy(), RetryStrategy.getDefaultExponential());
    } catch (Exception e) {
      // TODO: Might remove this try-catch block after revisited the constructors
      throw new RuntimeException("", e);
    }
  }

  /**
   * Returns a default policy that performs no retries, but invokes the action only once.
   *
   * @return a default policy that performs no retries, but invokes the action only once.
   */
  public static RetryPolicy getNoRetry() {
    return noRetry;
  }

  /**
   * Returns a default policy that implements a fixed retry interval configured with the default
   * {@link FixedInterval} retry strategy. The default retry policy treats all caught exceptions as
   * transient errors.
   *
   * @return a default policy that implements a fixed retry interval configured with the default
   *     {@link FixedInterval} retry strategy.
   */
  public static RetryPolicy getDefaultFixed() {
    return defaultFixed;
  }

  /**
   * Returns a default policy that implements a progressive retry interval configured with the
   * default {@link Incremental} retry strategy. The default retry policy treats all caught
   * exceptions as transient errors.
   *
   * @return a default policy that implements a progressive retry interval configured with the
   *     default {@link Incremental} retry strategy.
   */
  public static RetryPolicy getDefaultProgressive() {
    return defaultProgressive;
  }

  /**
   * Returns a default policy that implements a random exponential retry interval configured with
   * the default {@link FixedInterval} retry strategy. The default retry policy treats all caught
   * exceptions as transient errors.
   *
   * @return a default policy that implements a random exponential retry interval configured with
   *     the default {@link FixedInterval} retry strategy.
   */
  public static RetryPolicy getDefaultExponential() {
    return defaultExponential;
  }

  /**
   * Initializes a new instance of the {@link RetryPolicy} class with the specified number of retry
   * attempts and parameters defining the progressive delay between retries.
   *
   * @param errorDetectionStrategy The {@link ITransientErrorDetectionStrategy} that is responsible
   *     for detecting transient conditions.
   * @param retryStrategy The strategy to use for this retry policy.
   */
  public RetryPolicy(
      ITransientErrorDetectionStrategy errorDetectionStrategy, RetryStrategy retryStrategy) {
    Preconditions.checkNotNull(errorDetectionStrategy);
    Preconditions.checkNotNull(retryStrategy);

    this.setErrorDetectionStrategy(errorDetectionStrategy);
    this.setRetryStrategy(retryStrategy);
  }

  /**
   * Initializes a new instance of the {@link RetryPolicy} class with the specified number of retry
   * attempts and default fixed time interval between retries.
   *
   * @param errorDetectionStrategy The {@link ITransientErrorDetectionStrategy} that is responsible
   *     for detecting transient conditions.
   * @param retryCount The number of retry attempts.
   */
  public RetryPolicy(ITransientErrorDetectionStrategy errorDetectionStrategy, int retryCount) {
    this(errorDetectionStrategy, new FixedInterval(retryCount));
  }

  /**
   * Initializes a new instance of the {@link RetryPolicy} class with the specified number of retry
   * attempts and fixed time interval between retries.
   *
   * @param errorDetectionStrategy The {@link ITransientErrorDetectionStrategy} that is responsible
   *     for detecting transient conditions.
   * @param retryCount The number of retry attempts.
   * @param retryInterval The interval between retries.
   */
  public RetryPolicy(
      ITransientErrorDetectionStrategy errorDetectionStrategy,
      int retryCount,
      Duration retryInterval) {
    this(errorDetectionStrategy, new FixedInterval(retryCount, retryInterval));
  }

  /**
   * Initializes a new instance of the {@link RetryPolicy} class with the specified number of retry
   * attempts and backoff parameters for calculating the exponential delay between retries.
   *
   * @param errorDetectionStrategy The {@link ITransientErrorDetectionStrategy} that is responsible
   *     for detecting transient conditions.
   * @param retryCount The number of retry attempts.
   * @param minBackoff The minimum backoff time.
   * @param maxBackoff The maximum backoff time.
   * @param deltaBackoff The time value that will be used to calculate a random delta in the
   *     exponential delay between retries.
   */
  public RetryPolicy(
      ITransientErrorDetectionStrategy errorDetectionStrategy,
      int retryCount,
      Duration minBackoff,
      Duration maxBackoff,
      Duration deltaBackoff) {
    this(
        errorDetectionStrategy,
        new ExponentialBackoff(retryCount, minBackoff, maxBackoff, deltaBackoff));
  }

  /**
   * Initializes a new instance of the {@link RetryPolicy} class with the specified number of retry
   * attempts and parameters defining the progressive delay between retries.
   *
   * @param errorDetectionStrategy The {@link ITransientErrorDetectionStrategy} that is responsible
   *     for detecting transient conditions.
   * @param retryCount The number of retry attempts.
   * @param initialInterval The initial interval that will apply for the first retry.
   * @param increment The incremental time value that will be used to calculate the progressive
   *     delay between retries.
   */
  public RetryPolicy(
      ITransientErrorDetectionStrategy errorDetectionStrategy,
      int retryCount,
      Duration initialInterval,
      Duration increment) {
    this(errorDetectionStrategy, new Incremental(retryCount, initialInterval, increment));
  }

  /** Gets the retry strategy. */
  private RetryStrategy retryStrategy = null;

  public RetryStrategy getRetryStrategy() {
    return retryStrategy;
  }

  public void setRetryStrategy(RetryStrategy value) {
    retryStrategy = value;
  }

  /** Gets the instance of the error detection strategy. */
  private ITransientErrorDetectionStrategy errorDetectionStrategy;

  public ITransientErrorDetectionStrategy getErrorDetectionStrategy() {
    return errorDetectionStrategy;
  }

  public void setErrorDetectionStrategy(ITransientErrorDetectionStrategy value) {
    errorDetectionStrategy = value;
  }

  /**
   * Repetitively executes the specified action while it satisfies the current retry policy.
   *
   * @param action A delegate that represents the executable action that doesn't return any results.
   * @throws Exception The exception raised by <code>action</code>
   */
  public void executeAction(Runnable action) throws Exception {
    Preconditions.checkNotNull(action);

    this.executeAction(
        (Callable<Void>)
            () -> {
              action.run();
              return null;
            });
  }

  /**
   * Repetitively executes the specified action while it satisfies the current retry policy. The
   * type of result expected from the executable action.
   *
   * @param callable A delegate that represents the executable action that returns the result of
   *     type.
   * @param <T> The <code>callable</code>'s return type
   * @return The result from the action.
   * @throws Exception The exception raised by <code>callable</code>
   */
  public <T> T executeAction(Callable<T> callable) throws Exception {
    Preconditions.checkNotNull(callable);

    int retryCount = 0;
    Ref<Duration> delay = new Ref<>(null);

    ShouldRetry shouldRetry = this.getRetryStrategy().getShouldRetry();

    while (true) {
      try {
        return callable.call();
      } catch (RetryLimitExceededException limitExceededEx) {
        // The user code can throw a RetryLimitExceededException
        //     to force the exit from the retry loop.
        // The RetryLimitExceeded exception can have an inner exception attached to it.
        // This is the exception which we will have to throw up the stack
        //     so that callers can handle it.
        if (limitExceededEx.getInnerException() != null) {
          throw limitExceededEx.getInnerException();
        } else {
          return null;
        }
      } catch (Exception e) {
        if (!(this.getErrorDetectionStrategy().isTransient(e)
            && shouldRetry.invoke(retryCount++, e, delay))) {
          throw e;
        }
      }

      // Perform an extra check in the delay interval.
      // Should prevent from accidentally ending up with
      //     the value of -1 that will block a thread indefinitely.
      // In addition, any other negative numbers will cause an ArgumentOutOfRangeException fault
      //     that will be thrown by Thread.Sleep.
      if (delay.get().isNegative()) {
        delay.set(Duration.ZERO);
      }

      if (retryCount > 1 || !this.getRetryStrategy().getFastFirstRetry()) {
        Thread.sleep(delay.get().toMillis());
      }
    }
  }

  /**
   * Repetitively executes the specified asynchronous task in the given executor while it satisfies
   * the current retry policy.
   *
   * @param command the action to run before completing the returned {@link CompletableFuture}
   * @param executor the executor to use for asynchronous execution
   * @return A {@link CompletableFuture} that will run to completion if the given <code>command
   *     </code> completes successfully (either the first time or after retrying transient
   *     failures). If the task fails with a non-transient error or the retry limit is reached, the
   *     returned {@link CompletableFuture} will transition to a faulted state and the exception
   *     must be observed.
   */
  public CompletableFuture executeActionAsync(Runnable command, ScheduledExecutorService executor) {
    Preconditions.checkNotNull(command);

    Callable<Void> callable =
        () -> {
          command.run();
          return null;
        };
    return new AsyncExecution<Void>(
            callable,
            executor,
            getRetryStrategy().getShouldRetry(),
            getErrorDetectionStrategy(),
            getRetryStrategy().getFastFirstRetry())
        .executeAsync();
  }

  /**
   * Repeatedly executes the specified asynchronous task in the given executor while it satisfies
   * the current retry policy.
   *
   * @param command the {@link Callable} returning the value to be used to complete the returned
   *     {@link CompletableFuture}
   * @param executor the executor to use for asynchronous execution
   * @param <T> the {@link CompletableFuture}'s return type
   * @return A {@link CompletableFuture} that will run to completion if the given <code>command
   *     </code> completes successfully (either the first time or after retrying transient
   *     failures). If the task fails with a non-transient error or the retry limit is reached, the
   *     returned {@link CompletableFuture} will transition to a faulted state and the exception
   *     must be observed.
   */
  public <T> CompletableFuture<T> executeActionAsync(
      Callable<T> command, ScheduledExecutorService executor) {
    Preconditions.checkNotNull(command);

    return new AsyncExecution<T>(
            command,
            executor,
            getRetryStrategy().getShouldRetry(),
            getErrorDetectionStrategy(),
            getRetryStrategy().getFastFirstRetry())
        .executeAsync();
  }

  /** Implements a strategy that ignores any transient errors. */
  @VisibleForTesting
  static final class TransientErrorIgnoreStrategy implements ITransientErrorDetectionStrategy {
    /**
     * Always returns false.
     *
     * @param ex The exception.
     * @return Always false.
     */
    public boolean isTransient(Exception ex) {
      return false;
    }
  }

  /** Implements a strategy that treats all exceptions as transient errors. */
  @VisibleForTesting
  static final class TransientErrorCatchAllStrategy implements ITransientErrorDetectionStrategy {
    /**
     * Always returns true.
     *
     * @param ex The exception.
     * @return Always true.
     */
    public boolean isTransient(Exception ex) {
      return true;
    }
  }
}
