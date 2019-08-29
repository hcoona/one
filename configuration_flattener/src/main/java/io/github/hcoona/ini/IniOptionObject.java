package io.github.hcoona.ini;

import org.apache.commons.lang3.builder.CompareToBuilder;
import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.lang3.builder.ToStringBuilder;

import java.util.Map;

public class IniOptionObject<K, V>
    implements Map.Entry<K, V>, Comparable<IniOptionObject<K, V>> {
  private final K key;
  private V value;

  public IniOptionObject(K key, V value) {
    this.key = key;
    this.value = value;
  }

  @Override
  public K getKey() {
    return key;
  }

  @Override
  public V getValue() {
    return value;
  }

  @Override
  public V setValue(V value) {
    V oldValue = this.value;
    this.value = value;
    return oldValue;
  }

  @Override
  public int compareTo(IniOptionObject<K, V> o) {
    return new CompareToBuilder()
        .append(key, o.key)
        .append(value, o.value)
        .build();
  }

  @Override
  public boolean equals(Object obj) {
    if (obj == null) {
      return false;
    }
    if (obj == this) {
      return true;
    }
    if (!(obj instanceof IniOptionObject)) {
      return false;
    }
    IniOptionObject rhs = (IniOptionObject) obj;

    return new EqualsBuilder()
        .append(key, rhs.key)
        .append(value, rhs.value)
        .build();
  }

  @Override
  public int hashCode() {
    return new HashCodeBuilder()
        .append(key)
        .append(value)
        .toHashCode();
  }

  @Override
  public String toString() {
    return new ToStringBuilder(this)
        .append("key", key)
        .append("value", value)
        .build();
  }
}
