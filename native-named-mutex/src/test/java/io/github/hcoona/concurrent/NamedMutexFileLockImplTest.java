package io.github.hcoona.concurrent;

public class NamedMutexFileLockImplTest extends NamedMutexTest {
  @Override
  protected NamedMutex newNamedMutexInstance(boolean initiallyOwned, String name) throws Exception {
    return new NamedMutexFileLockImpl(initiallyOwned, name);
  }
}
