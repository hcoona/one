package io.github.hcoona.utils;

import com.google.common.annotations.VisibleForTesting;

@VisibleForTesting
interface Fcntl {
  interface Oflag {
    int O_CREAT = 0_100;
    int O_EXCL = 0_200;
  }
}
