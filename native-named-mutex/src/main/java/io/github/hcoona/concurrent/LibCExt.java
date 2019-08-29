package io.github.hcoona.concurrent;

import com.google.common.annotations.VisibleForTesting;
import com.sun.jna.Native;
import com.sun.jna.platform.unix.LibC;

@VisibleForTesting
interface LibCExt extends LibC {
  LibCExt INSTANCE = Native.loadLibrary("c", LibCExt.class);

  String strerror(int errnum);
}
