package io.github.hcoona.utils;

import com.sun.jna.Platform;
import java.util.concurrent.TimeUnit;

/**
 * A synchronization primitive that can also be used for inter-process synchronization.
 *
 * <p>Please use {@link io.github.hcoona.concurrent.NamedMutex} instead.
 */
@Deprecated
@SuppressWarnings("deprecation")
public abstract class NamedMutex implements AutoCloseable {
  /**
   * Create a platform specified implementation of NamedMutex.
   *
   * @param name the name of the mutex
   * @return A platform specified implementation of NamedMutex
   * @throws Exception Native errors
   */
  public static NamedMutex newInstance(String name) throws Exception {
    if (Platform.isWindows()) {
      return new NamedMutexWindowsImpl(name);
    } else if (Platform.isLinux()) {
      return new NamedMutexPosixImpl(name);
    } else {
      return new NamedMutexFileLockImpl(name);
    }
  }

  /**
   * Create a platform specified implementation of NamedMutex.
   *
   * @param initiallyOwned indicates whether the calling thread should have initial ownership of the
   *     mutex. If the mutex is already exist, this parameter is ignored.
   * @param name the name of the mutex
   * @return A platform specified implementation of NamedMutex
   * @throws Exception Native errors
   */
  public static NamedMutex newInstance(boolean initiallyOwned, String name) throws Exception {
    if (Platform.isWindows()) {
      return new NamedMutexWindowsImpl(initiallyOwned, name);
    } else if (Platform.isLinux()) {
      return new NamedMutexPosixImpl(initiallyOwned, name);
    } else {
      return new NamedMutexFileLockImpl(initiallyOwned, name);
    }
  }

  /**
   * Blocks the current thread until the current mutex receives a signal.
   *
   * @return true if the current instance receives a signal. If the current instance is never
   *     signaled, WaitOne never returns.
   * @throws Exception Native errors
   */
  public abstract boolean waitOne() throws Exception;

  /**
   * Blocks the current thread until the current mutex receives a signal or the waiting interval
   * arrives.
   *
   * @param interval The interval to wait
   * @param intervalTimeUnit The time unit of interval
   * @return true if the current instance receives a signal; otherwise, false.
   * @throws Exception Native errors
   */
  public abstract boolean waitOne(long interval, TimeUnit intervalTimeUnit) throws Exception;

  /**
   * Release the Mutex object once.
   *
   * @throws Exception Native errors
   */
  public abstract void release() throws Exception;

  /**
   * Release all resources used by current instance.
   *
   * @throws Exception Native errors when cleanup
   */
  @Override
  public abstract void close() throws Exception;

  @Override
  protected void finalize() throws Throwable {
    try {
      close();
    } finally {
      super.finalize();
    }
  }
}
