package io.github.hcoona.utils;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import org.junit.Assert;
import org.junit.Test;

@SuppressWarnings("deprecation")
public class NamedMutexTest {

  protected NamedMutex newNamedMutexInstance(boolean initiallyOwned, String name) throws Exception {
    return NamedMutex.newInstance(initiallyOwned, name);
  }

  @Test(timeout = 2000)
  public void testCannotWaitOne() throws Exception {
    final String name = "/test_named-mutex_cannot-wait-one";
    try (NamedMutex ignored = newNamedMutexInstance(true, name)) {
      ExecutorService executor = Executors.newSingleThreadExecutor();

      Future<Boolean> waitOneFuture =
          executor.submit(
              () -> {
                try (NamedMutex mutex2 = newNamedMutexInstance(true, name)) {
                  return mutex2.waitOne(500, TimeUnit.MILLISECONDS);
                }
              });

      Assert.assertFalse(waitOneFuture.get());
    }
  }

  @Test(timeout = 7000)
  public void testCanWaitOne() throws Exception {
    final String name = "/test_named-mutex_can-wait-one";
    try (NamedMutex mutex_owned = newNamedMutexInstance(true, name)) {
      mutex_owned.release();
      ExecutorService executor = Executors.newSingleThreadExecutor();

      Future<Boolean> waitOneFuture =
          executor.submit(
              () -> {
                try (NamedMutex mutex2 = newNamedMutexInstance(true, name)) {
                  return mutex2.waitOne(5, TimeUnit.SECONDS);
                }
              });

      Assert.assertTrue(waitOneFuture.get());
    } catch (PosixErrorException e) {
      System.err.println("Error code = " + e.getErrorCode());
    }
  }
}
