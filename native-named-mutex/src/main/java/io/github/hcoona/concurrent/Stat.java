package io.github.hcoona.concurrent;

import com.google.common.annotations.VisibleForTesting;

@VisibleForTesting
interface Stat {
  interface Mode {
    int S_IRWXU = 0_0700;
    int S_IRUSR = 0_0400;
    int S_IWUSR = 0_0200;
    int S_IXUSR = 0_0100;
    int S_IRWXG = 0_0070;
    int S_IRGRP = 0_0040;
  }
}
