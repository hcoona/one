package io.github.hcoona.retry;

import java.time.Duration;

public abstract class RetryStrategy {
  public static final int DefaultClientRetryCount = 10;

  public static final Duration DefaultClientBackoff = Duration.ofSeconds(10);
  public static final Duration DefaultMaxBackoff = Duration.ofSeconds(30);
  public static final Duration DefaultMinBackoff = Duration.ofSeconds(1);
  public static final Duration DefaultRetryInterval = Duration.ofSeconds(1);
  public static final Duration DefaultRetryIncrement = Duration.ofSeconds(1);

  public static final boolean DefaultFirstFastRetry = true;

  private static final RetryStrategy noRetry;
  private static final RetryStrategy defaultFixed;
  private static final RetryStrategy defaultProgressive;
  private static final RetryStrategy defaultExponential;

  static {
    noRetry = new FixedInterval(0, DefaultRetryInterval);
    defaultFixed = new FixedInterval(DefaultClientRetryCount, DefaultRetryInterval);
    defaultProgressive =
        new Incremental(DefaultClientRetryCount, DefaultRetryInterval, DefaultRetryIncrement);
    defaultExponential =
        new ExponentialBackoff(
            DefaultClientRetryCount, DefaultMinBackoff, DefaultMaxBackoff, DefaultClientBackoff);
  }

  /** @return a default policy that performs no retries, but invokes the action only once. */
  public static RetryStrategy getNoRetry() {
    return noRetry;
  }

  /**
   * Returns a default policy that implements a fixed retry interval configured with the {@link
   * #DefaultClientRetryCount} and {@link #DefaultRetryInterval} parameters. The default retry
   * policy treats all caught exceptions as transient errors.
   *
   * @return a default policy that implements a fixed retry interval configured with the {@link
   *     #DefaultClientRetryCount} and {@link #DefaultRetryInterval} parameters.
   */
  public static RetryStrategy getDefaultFixed() {
    return defaultFixed;
  }

  /**
   * Returns a default policy that implements a progressive retry interval configured with the
   * {@link #DefaultClientRetryCount}, {@link #DefaultRetryInterval}, and {@link
   * #DefaultRetryIncrement} parameters. The default retry policy treats all caught exceptions as
   * transient errors.
   *
   * @return a default policy that implements a progressive retry interval configured with the
   *     {@link #DefaultClientRetryCount}, {@link #DefaultRetryInterval}, and {@link
   *     #DefaultRetryIncrement} parameters.
   */
  public static RetryStrategy getDefaultProgressive() {
    return defaultProgressive;
  }

  /**
   * Returns a default policy that implements a random exponential retry interval configured with
   * the {@link #DefaultClientRetryCount}, {@link #DefaultMinBackoff}, {@link #DefaultMaxBackoff} ,
   * and {@link #DefaultClientBackoff} parameters. The default retry policy treats all caught
   * exceptions as transient errors.
   *
   * @return a default policy that implements a random exponential retry interval configured with
   *     the {@link #DefaultClientRetryCount}, {@link #DefaultMinBackoff}, {@link
   *     #DefaultMaxBackoff}, and {@link #DefaultClientBackoff} parameters.
   */
  public static RetryStrategy getDefaultExponential() {
    return defaultExponential;
  }

  /**
   * Initializes a new instance of the {@link #RetryStrategy} class.
   *
   * @param name The name of the retry strategy.
   * @param firstFastRetry true to immediately retry in the first attempt; otherwise, false. The
   *     subsequent retries will remain subject to the configured retry interval.
   */
  protected RetryStrategy(String name, boolean firstFastRetry) {
    this.setName(name);
    this.setFastFirstRetry(firstFastRetry);
  }

  /**
   * Gets or sets a value indicating whether the first retry attempt will be made immediately,
   * whereas subsequent retries will remain subject to the retry interval.
   */
  private boolean fastFirstRetry = false;

  public boolean getFastFirstRetry() {
    return fastFirstRetry;
  }

  public void setFastFirstRetry(boolean value) {
    fastFirstRetry = value;
  }

  /** Gets the name of the retry strategy. */
  private String name = "";

  public String getName() {
    return name;
  }

  public void setName(String value) {
    name = value;
  }

  /**
   * Returns the corresponding ShouldRetry delegate.
   *
   * @return The ShouldRetry delegate.
   */
  public abstract ShouldRetry getShouldRetry();
}
