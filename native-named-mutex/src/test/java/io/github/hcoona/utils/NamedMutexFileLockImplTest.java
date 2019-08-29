package io.github.hcoona.utils;

@SuppressWarnings("deprecation")
public class NamedMutexFileLockImplTest extends NamedMutexTest {
  @Override
  protected NamedMutex newNamedMutexInstance(boolean initiallyOwned, String name) throws Exception {
    return new NamedMutexFileLockImpl(initiallyOwned, name);
  }
}
