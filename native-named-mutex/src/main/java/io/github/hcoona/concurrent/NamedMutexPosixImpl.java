package io.github.hcoona.concurrent;

import com.google.common.annotations.VisibleForTesting;
import com.google.common.base.Preconditions;
import com.google.common.base.Strings;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import java.util.concurrent.TimeUnit;

@VisibleForTesting
class NamedMutexPosixImpl extends NamedMutex {
  private final String name;
  private Pointer handle;
  private boolean owned;
  private boolean disposed = false;
  private boolean unlinked = false;

  /**
   * Calling this constructor overload is the same as calling the {@link
   * #NamedMutexPosixImpl(boolean, String)} constructor overload and specifying false for initial
   * ownership of the mutex. That is, the calling thread does not own the mutex.
   *
   * @param name the name of the mutex
   * @throws PosixErrorException Posix native error
   */
  @VisibleForTesting
  NamedMutexPosixImpl(String name) throws PosixErrorException {
    this(false, name);
  }

  /**
   * This constructor initializes a Mutex object that represents a named system mutex. You can
   * create multiple Mutex objects that represent the same named system mutex.
   *
   * @param initiallyOwned indicates whether the calling thread should have initial ownership of the
   *     mutex
   * @param name the name of the mutex
   * @throws PosixErrorException Posix native error
   */
  @VisibleForTesting
  NamedMutexPosixImpl(boolean initiallyOwned, String name) throws PosixErrorException {
    Preconditions.checkArgument(!Strings.isNullOrEmpty(name), "name cannot be null or empty");
    Preconditions.checkArgument(name.charAt(0) == '/', "name must starts with /");

    this.name = name;
    handle =
        Pthread.INSTANCE.sem_open(
            name,
            Fcntl.Oflag.O_CREAT,
            Stat.Mode.S_IRWXU | Stat.Mode.S_IRUSR | Stat.Mode.S_IWUSR,
            initiallyOwned ? 0 : 1);

    if (handle == null) {
      throw new PosixErrorException(Native.getLastError());
    } else {
      IntByReference semaphoreValue = new IntByReference(0);
      int retCode = Pthread.INSTANCE.sem_getvalue(handle, semaphoreValue);
      if (retCode == 0) {
        owned = semaphoreValue.getValue() == 0;
      } else {
        throw new PosixErrorException(Native.getLastError());
      }
    }
  }

  @Override
  public boolean waitOne() throws Exception {
    Preconditions.checkState(!disposed);

    if (Pthread.INSTANCE.sem_wait(handle) == 0) {
      return true;
    } else {
      int errorCode = Native.getLastError();
      if (errorCode == Errno.EINTR) {
        throw new InterruptedException(LibCExt.INSTANCE.strerror(errorCode));
      } else {
        throw new PosixErrorException(Native.getLastError());
      }
    }
  }

  @Override
  public boolean waitOne(long interval, TimeUnit intervalTimeUnit) throws Exception {
    Preconditions.checkArgument(interval >= 0);
    Preconditions.checkState(!disposed);

    long seconds = intervalTimeUnit.toSeconds(interval);
    long nanoseconds = intervalTimeUnit.toNanos(interval) - TimeUnit.SECONDS.toNanos(seconds);
    Pthread.TimeSpecByReference timeout = new Pthread.TimeSpecByReference(seconds, nanoseconds);
    if (Pthread.INSTANCE.sem_timedwait(handle, timeout) == 0) {
      return true;
    } else {
      int errorCode = Native.getLastError();
      if (errorCode == Errno.ETIMEDOUT) {
        return false;
      } else if (errorCode == Errno.EINTR) {
        throw new InterruptedException(LibCExt.INSTANCE.strerror(errorCode));
      } else {
        throw new PosixErrorException(Native.getLastError());
      }
    }
  }

  @Override
  public void release() throws Exception {
    Preconditions.checkState(!disposed);

    if (owned) {
      if (Pthread.INSTANCE.sem_post(handle) == 0) {
        owned = false;
      } else {
        throw new PosixErrorException(Native.getLastError());
      }
    }
  }

  @Override
  public void close() throws Exception {
    if (!disposed) {
      release();
      if (handle != null) {
        if (Pthread.INSTANCE.sem_close(handle) == 0) {
          handle = null;
          disposed = true;
        } else {
          throw new PosixErrorException(Native.getLastError());
        }
      }
    }

    if (!unlinked) {
      if (Pthread.INSTANCE.sem_unlink(name) == 0) {
        unlinked = true;
      } else {
        int errorCode = Native.getLastError();
        if (errorCode != Errno.ENOENT) {
          throw new PosixErrorException(errorCode);
        }
      }
    }
  }
}
