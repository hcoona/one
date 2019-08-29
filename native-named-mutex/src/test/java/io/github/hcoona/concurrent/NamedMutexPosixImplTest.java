package io.github.hcoona.concurrent;

import com.sun.jna.Platform;
import java.util.concurrent.TimeUnit;
import org.junit.Assert;
import org.junit.Assume;
import org.junit.BeforeClass;
import org.junit.Test;

public class NamedMutexPosixImplTest {

  @BeforeClass
  public static void beforeClass() throws Exception {
    Assume.assumeTrue(Platform.isLinux());
  }

  @Test
  public void testNewInstance() throws Exception {
    try (NamedMutex mutex = NamedMutex.newInstance("/test_named-mutex_new-instance")) {
      Assert.assertTrue(mutex instanceof NamedMutexPosixImpl);
    }
  }

  @Test
  public void testCannotWait2() throws Exception {
    final String name = "/test_named-mutex_cannot-wait-2";
    try (NamedMutex mutex1 = NamedMutex.newInstance(true, name)) {
      try (NamedMutex mutex2 = NamedMutex.newInstance(false, name)) {
        Assert.assertFalse(mutex2.waitOne(500, TimeUnit.MILLISECONDS));
      } catch (PosixErrorException e) {
        System.err.println("Failed on wait, errno = " + e.getErrorCode());
      }
    }
  }
}
