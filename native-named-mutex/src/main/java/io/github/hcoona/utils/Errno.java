package io.github.hcoona.utils;

import com.google.common.annotations.VisibleForTesting;

@VisibleForTesting
interface Errno {
  int ENOENT = 2;
  int EINTR = 4;
  int EAGAIN = 11;
  int ETIMEDOUT = 110;
}
