package io.github.hcoona.ini;

import org.apache.commons.lang3.builder.CompareToBuilder;
import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.lang3.builder.ToStringBuilder;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

public class IniFileObject<TSection, TOptionKey, TOptionValue>
    implements Map<TSection, IniSectionObject<TSection, TOptionKey, TOptionValue>>,
    Comparable<IniFileObject<TSection, TOptionKey, TOptionValue>> {
  private final Map<TSection, IniSectionObject<TSection, TOptionKey, TOptionValue>>
      sectionMap;

  public IniFileObject(
      Map<TSection, IniSectionObject<TSection, TOptionKey, TOptionValue>> sectionMap) {
    this.sectionMap = sectionMap;
  }

  public IniSectionObject<TSection, TOptionKey, TOptionValue>
  put(IniSectionObject<TSection, TOptionKey, TOptionValue> value) {
    return sectionMap.put(value.getKey(), value);
  }

  @Override
  public int size() {
    return sectionMap.size();
  }

  @Override
  public boolean isEmpty() {
    return sectionMap.isEmpty();
  }

  @Override
  public boolean containsKey(Object key) {
    return sectionMap.containsKey(key);
  }

  @Override
  public boolean containsValue(Object value) {
    return sectionMap.containsValue(value);
  }

  @Override
  public IniSectionObject<TSection, TOptionKey, TOptionValue> get(Object key) {
    return sectionMap.get(key);
  }

  @Override
  public IniSectionObject<TSection, TOptionKey, TOptionValue>
  put(TSection key, IniSectionObject<TSection, TOptionKey, TOptionValue> value) {
    return sectionMap.put(key, value);
  }

  @Override
  public IniSectionObject<TSection, TOptionKey, TOptionValue> remove(Object key) {
    return sectionMap.remove(key);
  }

  @Override
  public void putAll(Map<? extends TSection, ? extends IniSectionObject<TSection, TOptionKey, TOptionValue>> m) {
    sectionMap.putAll(m);
  }

  @Override
  public void clear() {
    sectionMap.clear();
  }

  @Override
  public Set<TSection> keySet() {
    return sectionMap.keySet();
  }

  @Override
  public Collection<IniSectionObject<TSection, TOptionKey, TOptionValue>> values() {
    return sectionMap.values();
  }

  @Override
  public Set<Entry<TSection, IniSectionObject<TSection, TOptionKey, TOptionValue>>> entrySet() {
    return sectionMap.entrySet();
  }

  @Override
  public int compareTo(IniFileObject<TSection, TOptionKey, TOptionValue> o) {
    return new CompareToBuilder()
        .append(sectionMap, sectionMap)
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
    if (!(obj instanceof IniFileObject)) {
      return false;
    }
    IniFileObject rhs = (IniFileObject) obj;

    return new EqualsBuilder()
        .append(sectionMap, rhs.sectionMap)
        .build();
  }

  @Override
  public int hashCode() {
    return new HashCodeBuilder()
        .append(sectionMap)
        .toHashCode();
  }

  @Override
  public String toString() {
    return new ToStringBuilder(this)
        .append("sectionMap", sectionMap)
        .toString();
  }
}
