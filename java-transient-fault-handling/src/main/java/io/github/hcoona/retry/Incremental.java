package io.github.hcoona.retry;

import com.google.common.base.Preconditions;
import java.time.Duration;

/**
 * A retry strategy with a specified number of retry attempts and an incremental time interval
 * between retries.
 */
public class Incremental extends RetryStrategy {
  private final int retryCount;
  private final Duration initialInterval;
  private final Duration increment;

  /** Initializes a new instance of the {@link #Incremental} class. */
  public Incremental() {
    this(DefaultClientRetryCount, DefaultRetryInterval, DefaultRetryIncrement);
  }

  /**
   * Initializes a new instance of the {@link #Incremental} class with the specified retry settings.
   *
   * @param retryCount The number of retry attempts.
   * @param initialInterval The initial interval that will apply for the first retry.
   * @param increment The incremental time value that will be used to calculate the progressive
   *     delay between retries.
   */
  public Incremental(int retryCount, Duration initialInterval, Duration increment) {
    this(null, retryCount, initialInterval, increment);
  }

  /**
   * Initializes a new instance of the {@link #Incremental} class with the specified name and retry
   * settings.
   *
   * @param name The retry strategy name.
   * @param retryCount The number of retry attempts.
   * @param initialInterval The initial interval that will apply for the first retry.
   * @param increment The incremental time value that will be used to calculate the progressive
   *     delay between retries.
   */
  public Incremental(String name, int retryCount, Duration initialInterval, Duration increment) {
    this(name, retryCount, initialInterval, increment, DefaultFirstFastRetry);
  }

  /**
   * Initializes a new instance of the {@link #Incremental} class with the specified number of retry
   * attempts, time interval, retry strategy, and fast start option.
   *
   * @param name The retry strategy name.
   * @param retryCount The number of retry attempts.
   * @param initialInterval The initial interval that will apply for the first retry.
   * @param increment The incremental time value that will be used to calculate the progressive
   *     delay between retries.
   * @param firstFastRetry true to immediately retry in the first attempt; otherwise, false. The
   *     subsequent retries will remain subject to the configured retry interval.
   */
  public Incremental(
      String name,
      int retryCount,
      Duration initialInterval,
      Duration increment,
      boolean firstFastRetry) {
    super(name, firstFastRetry);

    Preconditions.checkArgument(retryCount >= 0);
    Preconditions.checkArgument(!initialInterval.isNegative());
    Preconditions.checkArgument(!increment.isNegative());

    this.retryCount = retryCount;
    this.initialInterval = initialInterval;
    this.increment = increment;
  }

  /**
   * Returns the corresponding ShouldRetry delegate.
   *
   * @return The ShouldRetry delegate.
   */
  public ShouldRetry getShouldRetry() {
    return (int currentRetryCount, Exception lastException, Ref<Duration> retryInterval) -> {
      if (currentRetryCount < this.retryCount) {
        retryInterval.set(
            Duration.ofMillis(
                this.initialInterval.toMillis() + (this.increment.toMillis() * currentRetryCount)));

        return true;
      }

      retryInterval.set(Duration.ZERO);

      return false;
    };
  }
}
