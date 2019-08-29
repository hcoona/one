package io.github.hcoona.util;

import java.io.Serializable;
import java.util.Locale;

public class CaseInsensitiveString implements
    Serializable,
    Comparable<CaseInsensitiveString>,
    CharSequence {
  private final String value;
  private final String lowerCasedValue;

  public CaseInsensitiveString(String value) {
    this(value, Locale.ROOT);
  }

  public CaseInsensitiveString(String value, Locale locale) {
    this.value = value;
    this.lowerCasedValue = value.toLowerCase(locale);
  }

  @Override
  public int length() {
    return value.length();
  }

  @Override
  public char charAt(int index) {
    return value.charAt(index);
  }

  @Override
  public CharSequence subSequence(int start, int end) {
    return value.subSequence(start, end);
  }

  public String getValue() {
    return value;
  }

  public String getLowerCasedValue() {
    return lowerCasedValue;
  }

  @Override
  public int compareTo(CaseInsensitiveString o) {
    return lowerCasedValue.compareTo(o.lowerCasedValue);
  }

  @Override
  public boolean equals(Object obj) {
    if (obj == null) {
      return false;
    }
    if (obj == this) {
      return true;
    }

    if (obj instanceof CaseInsensitiveString) {
      CaseInsensitiveString rhs = (CaseInsensitiveString) obj;
      return lowerCasedValue.equals(rhs.lowerCasedValue);
    } else if (obj instanceof String) {
      String rhs = (String) obj;
      return value.equalsIgnoreCase(rhs);
    } else {
      return false;
    }
  }

  @Override
  public int hashCode() {
    return lowerCasedValue.hashCode();
  }

  @Override
  public String toString() {
    return value;
  }
}
