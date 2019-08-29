package io.github.hcoona.utils;

import com.sun.jna.Native;
import com.sun.jna.platform.win32.Kernel32;
import com.sun.jna.platform.win32.WinBase;
import com.sun.jna.win32.W32APIOptions;

/** Interface definitions for <code>kernel32.dll</code>. Includes additional functions to Mutex */
interface Kernel32Ext extends Kernel32 {
  Kernel32Ext INSTANCE =
      Native.loadLibrary("kernel32", Kernel32Ext.class, W32APIOptions.DEFAULT_OPTIONS);

  /**
   * Creates or opens a named or unnamed mutex object.
   *
   * <p>To specify an access mask for the object, use the {@link #CreateMutexEx} function.
   *
   * @param lpMutexAttributes A pointer to a SECURITY_ATTRIBUTES structure. If this parameter is
   *     NULL, the handle cannot be inherited by child processes.
   * @param bInitialOwner If this value is TRUE and the caller created the mutex, the calling thread
   *     obtains initial ownership of the mutex object. Otherwise, the calling thread does not
   *     obtain ownership of the mutex. To determine if the caller created the mutex, see the Return
   *     Values section.
   * @param lpName The name of the mutex object. The name is limited to MAX_PATH characters. Name
   *     comparison is case sensitive.
   * @return If the function succeeds, the return value is a handle to the newly created mutex
   *     object. If the function fails, the return value is NULL. To get extended error information,
   *     call {@link Native#getLastError}. If the mutex is a named mutex and the object existed
   *     before this function call, the return value is a handle to the existing object,
   *     GetLastError returns ERROR_ALREADY_EXISTS, bInitialOwner is ignored, and the calling thread
   *     is not granted ownership. However, if the caller has limited access rights, the function
   *     will fail with ERROR_ACCESS_DENIED and the caller should use the OpenMutex function.
   */
  HANDLE CreateMutex(
      WinBase.SECURITY_ATTRIBUTES lpMutexAttributes, boolean bInitialOwner, String lpName);

  /**
   * Releases ownership of the specified mutex object.
   *
   * @param hMutex A handle to the mutex object. The {@link #CreateMutex} or {@link #OpenMutex}
   *     function returns this handle.
   * @return If the function succeeds, the return value is nonzero. If the function fails, the
   *     return value is zero. To get extended error information, call GetLastError.
   */
  boolean ReleaseMutex(HANDLE hMutex);
}
