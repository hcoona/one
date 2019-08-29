package io.github.hcoona.ini;

public class IniException extends RuntimeException {
  public IniException(String message) {
    super(message);
  }

  public IniException(String message, Throwable cause) {
    super(message, cause);
  }
}
