package io.github.hcoona.concurrent;

import com.sun.jna.LastErrorException;

/** Posix native exception. */
public class PosixErrorException extends LastErrorException {
  /** @param code Error code, usually should be global variable <code>errno</code>. */
  public PosixErrorException(int code) {
    super(code, LibCExt.INSTANCE.strerror(code));
  }
}
