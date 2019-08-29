package io.github.hcoona.directory_manifest;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.concurrent.Callable;
import java.util.concurrent.Delayed;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.RunnableScheduledFuture;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

public class LogErrorScheduledThreadPoolExecutor
    extends ScheduledThreadPoolExecutor {
  private static final Logger LOG =
      LoggerFactory.getLogger(LogErrorScheduledThreadPoolExecutor.class);

  public LogErrorScheduledThreadPoolExecutor(int corePoolSize) {
    super(corePoolSize);
  }

  public LogErrorScheduledThreadPoolExecutor(int corePoolSize, ThreadFactory threadFactory) {
    super(corePoolSize, threadFactory);
  }

  public LogErrorScheduledThreadPoolExecutor(int corePoolSize, RejectedExecutionHandler handler) {
    super(corePoolSize, handler);
  }

  public LogErrorScheduledThreadPoolExecutor(int corePoolSize, ThreadFactory threadFactory, RejectedExecutionHandler handler) {
    super(corePoolSize, threadFactory, handler);
  }

  @Override
  protected <V> RunnableScheduledFuture<V> decorateTask(Callable<V> callable, RunnableScheduledFuture<V> task) {
    return new RunnableScheduledFuture<V>() {
      @Override
      public boolean isPeriodic() {
        return task.isPeriodic();
      }

      @Override
      public long getDelay(TimeUnit unit) {
        return task.getDelay(unit);
      }

      @Override
      public int compareTo(Delayed o) {
        return task.compareTo(o);
      }

      @Override
      public void run() {
        task.run();
      }

      @Override
      public boolean cancel(boolean mayInterruptIfRunning) {
        return task.cancel(mayInterruptIfRunning);
      }

      @Override
      public boolean isCancelled() {
        return task.isCancelled();
      }

      @Override
      public boolean isDone() {
        return task.isDone();
      }

      @Override
      public V get() throws InterruptedException, ExecutionException {
        return task.get();
      }

      @Override
      public V get(long timeout, TimeUnit unit) throws InterruptedException, ExecutionException, TimeoutException {
        return task.get(timeout, unit);
      }

      @Override
      public String toString() {
        return callable.toString();
      }
    };
  }

  @Override
  protected void afterExecute(Runnable r, Throwable t) {
    super.afterExecute(r, t);
    logThrowableFromAfterExecute(r, t);
  }

  private static void logThrowableFromAfterExecute(Runnable r, Throwable t) {
    if (LOG.isDebugEnabled()) {
      LOG.debug("afterExecute in thread: " + Thread.currentThread().getName()
          + ", runnable type: " + r.getClass().getName()
          + ", runnable: " + r.toString());
    }

    //For additional information, see: https://docs.oracle
    // .com/javase/7/docs/api/java/util/concurrent/ThreadPoolExecutor
    // .html#afterExecute(java.lang.Runnable,%20java.lang.Throwable)

    // Handle JDK-8071638
    if (t == null && r instanceof Future<?> && ((Future<?>) r).isDone()) {
      try {
        ((Future<?>) r).get();
      } catch (ExecutionException ee) {
        LOG.warn("Execution exception when running task in "
            + Thread.currentThread().getName());
        t = ee.getCause();
      } catch (InterruptedException ie) {
        LOG.warn("Thread (" + Thread.currentThread() + ") interrupted: ", ie);
        Thread.currentThread().interrupt();
      } catch (Throwable throwable) {
        t = throwable;
      }
    }

    if (t != null) {
      LOG.warn("Caught exception in thread " + Thread
          .currentThread().getName() + ": ", t);
    }
  }
}
