package io.github.hcoona.retry;

import com.google.common.base.Preconditions;
import java.time.Duration;
import org.apache.commons.math3.util.FastMath;

/**
 * A retry strategy with backoff parameters for calculating the exponential delay between retries.
 */
public class ExponentialBackoff extends RetryStrategy {
  private static final double randomizationFactor = 0.2;
  private final int retryCount;
  private final Duration minBackoff;
  private final Duration maxBackoff;
  private final Duration deltaBackoff;

  /**
   * Initializes a new instance of the {@link #ExponentialBackoff} class.
   */
  public ExponentialBackoff() {
    this(DefaultClientRetryCount, DefaultMinBackoff, DefaultMaxBackoff, DefaultClientBackoff);
  }

  /**
   * Initializes a new instance of the {@link #ExponentialBackoff} class with the specified retry
   * settings.
   *
   * @param retryCount
   *     The maximum number of retry attempts.
   * @param minBackoff
   *     The minimum backoff time
   * @param maxBackoff
   *     The maximum backoff time.
   * @param deltaBackoff
   *     The value that will be used to calculate a random delta in the exponential delay between
   *     retries.
   */
  public ExponentialBackoff(
      int retryCount, Duration minBackoff, Duration maxBackoff, Duration deltaBackoff) {
    this(null, retryCount, minBackoff, maxBackoff, deltaBackoff, DefaultFirstFastRetry);
  }

  /**
   * Initializes a new instance of the
   * {@link #ExponentialBackoff}
   * class with the specified name and retry settings.
   *
   * @param name
   *     The name of the retry strategy.
   * @param retryCount
   *     The maximum number of retry attempts.
   * @param minBackoff
   *     The minimum backoff time
   * @param maxBackoff
   *     The maximum backoff time.
   * @param deltaBackoff
   *     The value that will be used to calculate a random delta in the exponential delay between
   *     retries.
   */
  public ExponentialBackoff(String name, int retryCount, Duration minBackoff, Duration maxBackoff,
      Duration deltaBackoff) {
    this(name, retryCount, minBackoff, maxBackoff, deltaBackoff, DefaultFirstFastRetry);
  }

  /**
   * Initializes a new instance of the {@link #ExponentialBackoff} class with the specified name,
   * retry settings, and fast retry option.
   *
   * @param name
   *     The name of the retry strategy.
   * @param retryCount
   *     The maximum number of retry attempts.
   * @param minBackoff
   *     The minimum backoff time
   * @param maxBackoff
   *     The maximum backoff time.
   * @param deltaBackoff
   *     The value that will be used to calculate a random delta in the exponential delay between
   *     retries.
   * @param firstFastRetry
   *     true to immediately retry in the first attempt; otherwise, false. The subsequent retries
   *     will remain subject to the configured retry interval.
   */
  public ExponentialBackoff(String name, int retryCount, Duration minBackoff, Duration maxBackoff,
      Duration deltaBackoff, boolean firstFastRetry) {
    super(name, firstFastRetry);

    Preconditions.checkArgument(retryCount >= 0);
    Preconditions.checkArgument(!minBackoff.isNegative());
    Preconditions.checkArgument(!maxBackoff.isNegative());
    Preconditions.checkArgument(!deltaBackoff.isNegative());
    Preconditions.checkArgument(minBackoff.compareTo(maxBackoff) <= 0);

    this.retryCount = retryCount;
    this.minBackoff = minBackoff;
    this.maxBackoff = maxBackoff;
    this.deltaBackoff = deltaBackoff;
  }

  /**
   * Returns the corresponding ShouldRetry delegate.
   *
   * @return The ShouldRetry delegate.
   */
  public ShouldRetry getShouldRetry() {
    return (int currentRetryCount, Exception lastException, Ref<Duration> retryInterval) -> {
      if (currentRetryCount < this.retryCount) {
        long deltaMillis = (long) ((FastMath.pow(2, currentRetryCount) - 1)
            * getRandomRange(deltaBackoff.toMillis() * (1 - randomizationFactor),
                deltaBackoff.toMillis() * (1 + randomizationFactor)));

        retryInterval.set(min(minBackoff.plusMillis(deltaMillis), maxBackoff));

        return true;
      }

      retryInterval.set(Duration.ZERO);
      return false;
    };
  }

  private static Duration min(Duration lhs, Duration rhs) {
    return lhs.compareTo(rhs) < 0 ? lhs : rhs;
  }

  private static double getRandomRange(double minValueIncluded, double maxValueExcluded) {
    return minValueIncluded + Math.random() * (maxValueExcluded - minValueIncluded + 1);
  }
}
