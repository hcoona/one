package io.github.hcoona.retry;

public class Ref<T> {
  private T value;

  public Ref(T value) {
    this.value = value;
  }

  public T get() {
    return value;
  }

  public void set(T value) {
    this.value = value;
  }

  @Override
  public String toString() {
    return value.toString();
  }

  @Override
  @SuppressWarnings("EqualsWhichDoesntCheckParameterClass")
  public boolean equals(Object obj) {
    return value.equals(obj);
  }

  @Override
  public int hashCode() {
    return value.hashCode();
  }
}
