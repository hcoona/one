package io.github.hcoona.retry;

/**
 * The special type of exception that provides managed exit from a retry loop. The user code can use
 * this exception to notify the retry policy that no further retry attempts are required.
 */
public final class RetryLimitExceededException extends Exception {
  /**
   * Initializes a new instance of the
   * {@link RetryLimitExceededException}
   * class with a default error message.
   */
  public RetryLimitExceededException() {
    this("The action has exceeded its defined retry limit.");
  }

  /**
   * Initializes a new instance of the
   * {@link RetryLimitExceededException}
   * class with a specified error message.
   *
   * @param message
   *     The message that describes the error.
   */
  public RetryLimitExceededException(String message) {
    super(message);
  }

  /**
   * Initializes a new instance of the
   * {@link RetryLimitExceededException}
   * class with a reference to the inner exception
   * that is the cause of this exception.
   *
   * @param innerException
   *     The exception that is the cause of the current exception.
   */
  public RetryLimitExceededException(Exception innerException) {
    super(innerException != null ? innerException.getMessage()
                                 : "The action has exceeded its defined retry limit.",
        innerException);
  }

  /**
   * Initializes a new instance of the
   * {@link RetryLimitExceededException}
   * class with a specified error message and inner exception.
   *
   * @param message
   *     The message that describes the error.
   * @param innerException
   *     The exception that is the cause of the current exception.
   */
  public RetryLimitExceededException(String message, Exception innerException) {
    super(message, innerException);
  }

  /**
   * Return the inner exception who caused {@link RetryLimitExceededException}.
   *
   * @return The inner exception
   */
  @SuppressWarnings("unchecked")
  public Exception getInnerException() {
    return (Exception) getCause();
  }
}
