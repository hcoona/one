package io.github.hcoona.concurrent;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.platform.win32.WinBase;
import com.sun.jna.platform.win32.WinError;
import com.sun.jna.platform.win32.WinNT;
import java.io.InvalidObjectException;
import java.util.concurrent.TimeUnit;

@VisibleForTesting
class NamedMutexWindowsImpl extends NamedMutex {
  private WinNT.HANDLE handle;
  private boolean owned;
  private boolean disposed = false;

  /**
   * Calling this constructor overload is the same as calling the {@link
   * #NamedMutexWindowsImpl(boolean, String)} constructor overload and specifying false for initial
   * ownership of the mutex. That is, the calling thread does not own the mutex.
   *
   * @param name the name of the mutex
   * @throws Win32Exception Windows native error
   */
  @VisibleForTesting
  NamedMutexWindowsImpl(String name) throws Win32Exception {
    this(false, name);
  }

  /**
   * This constructor initializes a Mutex object that represents a named system mutex. You can
   * create multiple Mutex objects that represent the same named system mutex.
   *
   * @param initiallyOwned indicates whether the calling thread should have initial ownership of the
   *     mutex
   * @param name the name of the mutex
   * @throws Win32Exception Windows native error
   */
  @VisibleForTesting
  NamedMutexWindowsImpl(boolean initiallyOwned, String name) throws Win32Exception {
    Preconditions.checkArgument(!Strings.isNullOrEmpty(name), "name cannot be null or empty");

    handle = Kernel32Ext.INSTANCE.CreateMutex(null, initiallyOwned, name);

    int errorCode = Kernel32Ext.INSTANCE.GetLastError();
    if (errorCode == 0) {
      owned = initiallyOwned;
    } else {
      if (errorCode == WinError.ERROR_ALREADY_EXISTS) {
        owned = false;
      } else {
        throw new Win32Exception(errorCode);
      }
    }
  }

  @Override
  public boolean waitOne() throws Exception {
    return waitOneInternal(WinBase.INFINITE);
  }

  @Override
  public boolean waitOne(long interval, TimeUnit intervalTimeUnit) throws Exception {
    return waitOneInternal((int) intervalTimeUnit.toMillis(interval));
  }

  private boolean waitOneInternal(int intervalMillis)
      throws AbandonedMutexException, InvalidObjectException {
    Preconditions.checkArgument(intervalMillis >= -1);
    Preconditions.checkState(!disposed);

    int returnCode = Kernel32Ext.INSTANCE.WaitForSingleObject(handle, intervalMillis);

    if (returnCode == WinBase.WAIT_ABANDONED) {
      throw new AbandonedMutexException();
    }

    return returnCode != WinError.WAIT_TIMEOUT;
  }

  @Override
  public void release() throws Exception {
    Preconditions.checkState(!disposed);

    if (owned) {
      if (Kernel32Ext.INSTANCE.ReleaseMutex(handle)) {
        owned = false;
      } else {
        throw new Win32Exception(Kernel32Ext.INSTANCE.GetLastError());
      }
    }
  }

  @Override
  public void close() throws Exception {
    if (!disposed) {
      release();
      if (Kernel32Ext.INSTANCE.CloseHandle(handle)) {
        handle = null;
        disposed = true;
      } else {
        throw new Win32Exception(Kernel32Ext.INSTANCE.GetLastError());
      }
    }
  }
}
