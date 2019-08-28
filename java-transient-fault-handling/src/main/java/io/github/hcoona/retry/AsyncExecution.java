package io.github.hcoona.retry;

import com.google.common.annotations.VisibleForTesting;
import java.time.Duration;
import java.util.concurrent.Callable;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

@VisibleForTesting
class AsyncExecution<T> implements Runnable {
  private final Callable<T> command;
  private final ScheduledExecutorService executor;
  private final ShouldRetry shouldRetry;
  private final ITransientErrorDetectionStrategy errorDetectionStrategy;
  private final boolean fastFirstRetry;
  private final CompletableFuture<T> future;

  private int retryCount = 0;
  private final Ref<Duration> delay = new Ref<Duration>(null);

  public AsyncExecution(Callable<T> command, ScheduledExecutorService executor,
      ShouldRetry shouldRetry, ITransientErrorDetectionStrategy errorDetectionStrategy,
      boolean fastFirstRetry) {
    this.command = command;
    this.executor = executor;
    this.shouldRetry = shouldRetry;
    this.errorDetectionStrategy = errorDetectionStrategy;
    this.fastFirstRetry = fastFirstRetry;
    this.future = new CompletableFuture<T>();
  }

  public CompletableFuture<T> executeAsync() {
    executor.execute(this);
    return future;
  }

  @Override
  public void run() {
    try {
      T result = command.call();
      future.complete(result);
    } catch (RetryLimitExceededException limitExceededEx) {
      // The user code can throw a RetryLimitExceededException
      //     to force the exit from the retry loop.
      // The RetryLimitExceeded exception can have an inner exception attached to it.
      // This is the exception which we will have to throw up the stack
      //     so that callers can handle it.
      if (limitExceededEx.getInnerException() != null) {
        future.completeExceptionally(limitExceededEx.getInnerException());
      } else {
        future.complete(null);
      }
    } catch (Exception e) {
      if (!(errorDetectionStrategy.isTransient(e) && shouldRetry.invoke(retryCount++, e, delay))) {
        future.completeExceptionally(e);
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

    if (retryCount > 1 || !fastFirstRetry) {
      executor.schedule(this, delay.get().getNano(), TimeUnit.NANOSECONDS);
    } else {
      executor.execute(this);
    }
  }
}
