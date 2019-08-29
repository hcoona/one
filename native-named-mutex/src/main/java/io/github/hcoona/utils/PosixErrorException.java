package io.github.hcoona.utils;

import com.sun.jna.LastErrorException;

/**
 * Posix native exception.
 *
 * <p>Please use {@link io.github.hcoona.concurrent.PosixErrorException} instead.
 */
@Deprecated
public class PosixErrorException extends LastErrorException {
  /** @param code Error code, usually should be global variable <code>errno</code>. */
  public PosixErrorException(int code) {
    super(code, LibCExt.INSTANCE.strerror(code));
  }
}
