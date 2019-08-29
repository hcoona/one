package io.github.hcoona.utils;

import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.platform.win32.WinBase;

/**
 * The exception that is thrown when one thread acquires a Mutex object that another thread has
 * abandoned by exiting without releasing it.
 *
 * <p>Please use {@link io.github.hcoona.concurrent.AbandonedMutexException} instead.
 */
@Deprecated
public class AbandonedMutexException extends Win32Exception {
  public AbandonedMutexException() {
    super(WinBase.WAIT_ABANDONED);
  }
}
