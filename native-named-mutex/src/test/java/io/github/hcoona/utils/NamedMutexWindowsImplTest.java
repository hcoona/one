package io.github.hcoona.utils;

import com.sun.jna.Platform;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import org.apache.commons.exec.CommandLine;
import org.apache.commons.exec.DefaultExecutor;
import org.junit.Assert;
import org.junit.Assume;
import org.junit.BeforeClass;
import org.junit.Test;

@SuppressWarnings("deprecation")
public class NamedMutexWindowsImplTest {

  @BeforeClass
  public static void beforeClass() throws Exception {
    Assume.assumeTrue(Platform.isWindows());
  }

  @Test
  public void testNewInstance() throws Exception {
    try (NamedMutex mutex = NamedMutex.newInstance("test_named-mutex_new-instance")) {
      Assert.assertTrue(mutex instanceof NamedMutexWindowsImpl);
    }
  }

  @Test
  public void testInterop() throws Exception {
    final String name = "test_named-mutex_interop";
    final int powershellWaitSeconds = 8;
    CommandLine commandLine =
        CommandLine.parse(
            "PowerShell.exe -Command \"[System.Threading.Mutex]::new($True, \'"
                + name
                + "\'); "
                + "Start-Sleep -Seconds "
                + powershellWaitSeconds
                + "\"");
    DefaultExecutor cmdExecutor = new DefaultExecutor();

    ExecutorService executor = Executors.newSingleThreadExecutor();
    Future<Integer> powershellMutexFuture = executor.submit(() -> cmdExecutor.execute(commandLine));

    Thread.sleep(TimeUnit.SECONDS.toMillis(powershellWaitSeconds) / 2);
    try (NamedMutex mutex = NamedMutex.newInstance(name)) {
      Assert.assertFalse(mutex.waitOne(500, TimeUnit.MILLISECONDS));
    }

    Assert.assertEquals(0, powershellMutexFuture.get().intValue());
  }

  @Test(expected = AbandonedMutexException.class, timeout = 3000)
  public void testAbandoned() throws Exception {
    final String name = "test_named-mutex_abandoned";

    ExecutorService executor = Executors.newSingleThreadExecutor();
    Future<?> future = executor.submit(() -> NamedMutex.newInstance(true, name));
    future.get();
    executor.shutdown();

    try (NamedMutex mutex = NamedMutex.newInstance(name)) {
      mutex.waitOne();
    }
  }
}
