package io.github.hcoona.utils;

import com.sun.jna.Native;
import com.sun.jna.Platform;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import org.junit.Assert;
import org.junit.Assume;
import org.junit.BeforeClass;
import org.junit.Test;

public class PthreadTest {

  @BeforeClass
  public static void beforeClass() throws Exception {
    Assume.assumeTrue(Platform.isLinux());
  }

  @Test
  public void testSelfPid() throws Exception {
    Assert.assertNotEquals(0, Pthread.INSTANCE.pthread_self());
  }

  @Test(timeout = 5000)
  public void testSemaphoreBasic() throws Exception {
    final String name = "/test_semaphore_basic";

    Pointer sem =
        Pthread.INSTANCE.sem_open(
            name,
            Fcntl.Oflag.O_CREAT,
            Stat.Mode.S_IRWXU | Stat.Mode.S_IRUSR | Stat.Mode.S_IWUSR,
            0);
    if (sem == null) {
      Assert.fail(LibCExt.INSTANCE.strerror(Native.getLastError()));
    } else {
      Assert.assertEquals(-1, Pthread.INSTANCE.sem_trywait(sem));
      Assert.assertEquals(Errno.EAGAIN, Native.getLastError());
      Assert.assertEquals(0, Pthread.INSTANCE.sem_post(sem));
      IntByReference value = new IntByReference(0);
      Assert.assertEquals(0, Pthread.INSTANCE.sem_getvalue(sem, value));
      Assert.assertEquals(1, value.getValue());
      Assert.assertEquals(0, Pthread.INSTANCE.sem_trywait(sem));
      Assert.assertEquals(-1, Pthread.INSTANCE.sem_trywait(sem));
      Assert.assertEquals(Errno.EAGAIN, Native.getLastError());
      Pthread.TimeSpecByReference timeout = new Pthread.TimeSpecByReference(1, 0);
      Assert.assertEquals(-1, Pthread.INSTANCE.sem_timedwait(sem, timeout));
      Assert.assertEquals(Errno.ETIMEDOUT, Native.getLastError());
      Assert.assertEquals(0, Pthread.INSTANCE.sem_close(sem));
      Assert.assertEquals(0, Pthread.INSTANCE.sem_unlink(name));
    }
  }

  @Test(timeout = 5000)
  public void testSemaphoreNamed() throws Exception {
    final String name = "/test_semaphore_basic";

    Pointer sem1 =
        Pthread.INSTANCE.sem_open(
            name,
            Fcntl.Oflag.O_CREAT,
            Stat.Mode.S_IRWXU | Stat.Mode.S_IRUSR | Stat.Mode.S_IWUSR,
            0);
    Assert.assertNotNull(sem1);

    Pointer sem2 = Pthread.INSTANCE.sem_open(name, Fcntl.Oflag.O_CREAT);
    Assert.assertNotNull(sem2);

    Assert.assertEquals(-1, Pthread.INSTANCE.sem_trywait(sem2));
    Assert.assertEquals(0, Pthread.INSTANCE.sem_post(sem1));
    Assert.assertEquals(0, Pthread.INSTANCE.sem_trywait(sem2));

    Assert.assertEquals(0, Pthread.INSTANCE.sem_close(sem1));
    Assert.assertEquals(0, Pthread.INSTANCE.sem_close(sem2));

    Assert.assertEquals(0, Pthread.INSTANCE.sem_unlink(name));
  }
}
