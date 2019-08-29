package io.github.hcoona.retry;

import com.google.common.base.Preconditions;
import com.google.common.collect.Maps;
import java.util.HashMap;
import java.util.Map;
import org.apache.commons.lang3.StringUtils;

/** Provides the entry point to the retry functionality. */
public class RetryManager {
  private static RetryManager defaultRetryManager;

  private final Map<String, RetryStrategy> retryStrategies;
  private final Map<String, String> defaultRetryStrategyNamesMap;
  private final Map<String, RetryStrategy> defaultRetryStrategiesMap;

  private String defaultRetryStrategyName;
  private RetryStrategy defaultStrategy;

  /**
   * Sets the specified retry manager as the default retry manager.
   *
   * @param retryManager The retry manager.
   * @param throwIfSet true to throw an exception if the manager is already set; otherwise, false.
   *     Defaults to <code>true</code>.
   * @throws IllegalStateException The singleton is already set and {@code throwIfSet} is true.
   */
  public static void setDefault(RetryManager retryManager, boolean throwIfSet)
      throws IllegalStateException {
    if (defaultRetryManager != null && throwIfSet && retryManager != defaultRetryManager) {
      throw new IllegalStateException("The RetryManager is already set.");
    }

    defaultRetryManager = retryManager;
  }

  /**
   * Gets the default {@link RetryManager} for the application. You can update the default retry
   * manager by calling the {@link RetryManager#setDefault(RetryManager, boolean)} method.
   *
   * @return The default {@link RetryManager}
   */
  public static RetryManager getInstance() throws IllegalStateException {
    RetryManager instance = defaultRetryManager;
    if (instance == null) {
      throw new IllegalStateException(
          "The default RetryManager has not been set. Set it by invoking the RetryManager. "
              + "SetDefault static method, or if you are using declarative configuration, you can "
              + "invoke the RetryPolicyFactory.CreateDefault() method to automatically create the "
              + "retry manager from the configuration file.");
    }

    return instance;
  }

  /**
   * Initializes a new instance of the {@link RetryManager} class.
   *
   * @param retryStrategies The complete set of retry strategies.
   */
  public RetryManager(Iterable<RetryStrategy> retryStrategies) {
    this(retryStrategies, null, null);
  }

  /**
   * Initializes a new instance of the {@link RetryManager} class with the specified retry
   * strategies and default retry strategy name.
   *
   * @param retryStrategies The complete set of retry strategies.
   * @param defaultRetryStrategyName The default retry strategy.
   */
  public RetryManager(Iterable<RetryStrategy> retryStrategies, String defaultRetryStrategyName) {
    this(retryStrategies, defaultRetryStrategyName, null);
  }

  /**
   * Initializes a new instance of the {@link RetryManager} class with the specified retry
   * strategies and defaults.
   *
   * @param retryStrategies The complete set of retry strategies.
   * @param defaultRetryStrategyName The default retry strategy.
   * @param defaultRetryStrategyNamesMap The names of the default strategies for different
   *     technologies.
   */
  public RetryManager(
      Iterable<RetryStrategy> retryStrategies,
      String defaultRetryStrategyName,
      Map<String, String> defaultRetryStrategyNamesMap) {
    this.retryStrategies = Maps.uniqueIndex(retryStrategies, RetryStrategy::getName);
    this.defaultRetryStrategyNamesMap = defaultRetryStrategyNamesMap;
    this.setDefaultRetryStrategyName(defaultRetryStrategyName);

    this.defaultRetryStrategiesMap = new HashMap<>();
    if (this.defaultRetryStrategyNamesMap != null) {
      this.defaultRetryStrategyNamesMap.entrySet().stream()
          .filter(p -> StringUtils.isNotBlank(p.getValue()))
          .forEach(
              map -> {
                RetryStrategy retryStrategy = this.retryStrategies.get(map.getValue());
                this.defaultRetryStrategiesMap.put(map.getKey(), retryStrategy);
              });
    }
  }

  /**
   * Get the default retry strategy name.
   *
   * @return the default retry strategy name.
   */
  public String getDefaultRetryStrategyName() {
    return this.defaultRetryStrategyName;
  }

  /**
   * Set the default retry strategy name.
   *
   * @param value the default retry strategy name
   */
  public void setDefaultRetryStrategyName(String value) {
    if (StringUtils.isNotBlank(value)) {
      RetryStrategy strategy = this.retryStrategies.get(value);
      this.defaultRetryStrategyName = value;
      this.defaultStrategy = strategy;
    } else {
      this.defaultRetryStrategyName = null;
    }
  }

  /**
   * Returns a retry policy with the specified error detection strategy and the default retry
   * strategy defined in the configuration. The type that implements the {@link
   * ITransientErrorDetectionStrategy} interface that is responsible for detecting transient
   * conditions.
   *
   * @param errorDetectionStrategy The error detection strategy.
   * @return A new retry policy with the specified error detection strategy and the default retry
   *     strategy defined in the configuration.
   */
  public RetryPolicy getRetryPolicy(ITransientErrorDetectionStrategy errorDetectionStrategy) {
    return new RetryPolicy(errorDetectionStrategy, this.getRetryStrategy());
  }

  /**
   * Returns a retry policy with the specified error detection strategy and retry strategy. The type
   * that implements the {@link ITransientErrorDetectionStrategy} interface that is responsible for
   * detecting transient conditions.
   *
   * @param errorDetectionStrategy The error detection strategy.
   * @param retryStrategyName The retry strategy name, as defined in the configuration.
   * @return A new retry policy with the specified error detection strategy and the default retry
   *     strategy defined in the configuration.
   */
  public RetryPolicy getRetryPolicy(
      ITransientErrorDetectionStrategy errorDetectionStrategy, String retryStrategyName) {
    return new RetryPolicy(errorDetectionStrategy, this.getRetryStrategy(retryStrategyName));
  }

  /**
   * Returns the default retry strategy defined in the configuration.
   *
   * @return The retry strategy that matches the default strategy.
   */
  public RetryStrategy getRetryStrategy() {
    return this.defaultStrategy;
  }

  /**
   * Returns the retry strategy that matches the specified name.
   *
   * @param retryStrategyName The retry strategy name.
   * @return The retry strategy that matches the specified name.
   */
  public RetryStrategy getRetryStrategy(String retryStrategyName) {
    Preconditions.checkArgument(StringUtils.isNotBlank(retryStrategyName));

    return this.retryStrategies.get(retryStrategyName);
  }

  /**
   * Returns the retry strategy for the specified technology.
   *
   * @param technology The techonolgy to get the default retry strategy for.
   * @return The retry strategy for the specified technology.
   */
  public RetryStrategy getDefaultRetryStrategy(String technology) {
    Preconditions.checkArgument(StringUtils.isNotBlank(technology));

    return this.defaultRetryStrategiesMap.get(technology);
  }
}
