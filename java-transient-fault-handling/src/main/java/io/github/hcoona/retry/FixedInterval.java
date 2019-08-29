package io.github.hcoona.retry;

import com.google.common.base.Preconditions;
import java.time.Duration;

/**
 * Represents a retry strategy with a specified number of retry attempts and a default, fixed time
 * interval between retries.
 */
public class FixedInterval extends RetryStrategy {
  private final int retryCount;
  private final Duration retryInterval;

  /** Initializes a new instance of the {@link #FixedInterval} class. */
  public FixedInterval() {
    this(DefaultClientRetryCount);
  }

  /**
   * Initializes a new instance of the {@link #FixedInterval} class with the specified number of
   * retry attempts.
   *
   * @param retryCount The number of retry attempts.
   */
  public FixedInterval(int retryCount) {
    this(retryCount, DefaultRetryInterval);
  }

  /**
   * Initializes a new instance of the {@link #FixedInterval} class with the specified number of
   * retry attempts and time interval.
   *
   * @param retryCount The number of retry attempts.
   * @param retryInterval The time interval between retries.
   */
  public FixedInterval(int retryCount, Duration retryInterval) {
    this(null, retryCount, retryInterval, DefaultFirstFastRetry);
  }

  /**
   * Initializes a new instance of the {@link #FixedInterval} class with the specified number of
   * retry attempts, time interval, and retry strategy.
   *
   * @param name The retry strategy name.
   * @param retryCount The number of retry attempts.
   * @param retryInterval The time interval between retries.
   */
  public FixedInterval(String name, int retryCount, Duration retryInterval) {
    this(name, retryCount, retryInterval, DefaultFirstFastRetry);
  }

  /**
   * Initializes a new instance of the {@link #FixedInterval} class with the specified number of
   * retry attempts, time interval, retry strategy, and fast start option.
   *
   * @param name The retry strategy name.
   * @param retryCount The number of retry attempts.
   * @param retryInterval The time interval between retries.
   * @param firstFastRetry true to immediately retry in the first attempt; otherwise, false. The
   *     subsequent retries will remain subject to the configured retry interval.
   */
  public FixedInterval(
      String name, int retryCount, Duration retryInterval, boolean firstFastRetry) {
    super(name, firstFastRetry);

    Preconditions.checkArgument(retryCount >= 0);
    Preconditions.checkArgument(!retryInterval.isNegative());

    this.retryCount = retryCount;
    this.retryInterval = retryInterval;
  }

  /**
   * Returns the corresponding ShouldRetry delegate.
   *
   * @return The ShouldRetry delegate.
   */
  public ShouldRetry getShouldRetry() {
    if (this.retryCount == 0) {
      return (int currentRetryCount, Exception lastException, Ref<Duration> interval) -> {
        interval.set(Duration.ZERO);
        return false;
      };
    }

    return (int currentRetryCount, Exception lastException, Ref<Duration> interval) -> {
      if (currentRetryCount < this.retryCount) {
        interval.set(this.retryInterval);
        return true;
      }

      interval.set(Duration.ZERO);
      return false;
    };
  }
}
