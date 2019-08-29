package io.github.hcoona.ini;

import org.apache.commons.lang3.builder.CompareToBuilder;
import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.apache.commons.lang3.builder.ToStringBuilder;

import java.util.Collection;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

public class IniSectionObject<TSection, TOptionKey, TOptionValue>
    implements Map.Entry<TSection, Set<IniOptionObject<TOptionKey, TOptionValue>>>,
    Set<IniOptionObject<TOptionKey, TOptionValue>>,
    Comparable<IniSectionObject<TSection, TOptionKey, TOptionValue>> {
  private final TSection section;
  private Set<IniOptionObject<TOptionKey, TOptionValue>> optionSet;

  public IniSectionObject(TSection section, Set<IniOptionObject<TOptionKey, TOptionValue>> optionSet) {
    this.section = section;
    this.optionSet = optionSet;
  }

  @Override
  public TSection getKey() {
    return section;
  }

  @Override
  public Set<IniOptionObject<TOptionKey, TOptionValue>> getValue() {
    return optionSet;
  }

  @Override
  public Set<IniOptionObject<TOptionKey, TOptionValue>> setValue(Set<IniOptionObject<TOptionKey, TOptionValue>> value) {
    final Set<IniOptionObject<TOptionKey, TOptionValue>> oldValue = this.optionSet;
    this.optionSet = value;
    return oldValue;
  }

  @Override
  public int size() {
    return optionSet.size();
  }

  @Override
  public boolean isEmpty() {
    return optionSet.isEmpty();
  }

  @Override
  public boolean contains(Object o) {
    return optionSet.contains(o);
  }

  @Override
  public Iterator<IniOptionObject<TOptionKey, TOptionValue>> iterator() {
    return optionSet.iterator();
  }

  @Override
  public Object[] toArray() {
    return optionSet.toArray();
  }

  @SuppressWarnings("SuspiciousToArrayCall")
  @Override
  public <T> T[] toArray(T[] a) {
    return optionSet.toArray(a);
  }

  @Override
  public boolean add(IniOptionObject<TOptionKey, TOptionValue> tOptionKeyTOptionValueIniOptionObject) {
    return optionSet.add(tOptionKeyTOptionValueIniOptionObject);
  }

  @Override
  public boolean remove(Object o) {
    return optionSet.remove(o);
  }

  @Override
  public boolean containsAll(Collection<?> c) {
    return optionSet.containsAll(c);
  }

  @Override
  public boolean addAll(Collection<? extends IniOptionObject<TOptionKey, TOptionValue>> c) {
    return optionSet.addAll(c);
  }

  @Override
  public boolean retainAll(Collection<?> c) {
    return optionSet.retainAll(c);
  }

  @Override
  public boolean removeAll(Collection<?> c) {
    return optionSet.removeAll(c);
  }

  @Override
  public void clear() {
    optionSet.clear();
  }

  @Override
  public int compareTo(IniSectionObject<TSection, TOptionKey, TOptionValue> o) {
    return new CompareToBuilder()
        .append(section, o.section)
        .append(optionSet, o.optionSet)
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
    if (!(obj instanceof IniSectionObject)) {
      return false;
    }
    IniSectionObject rhs = (IniSectionObject) obj;

    return new EqualsBuilder()
        .append(section, rhs.section)
        .append(optionSet, rhs.optionSet)
        .build();
  }

  @Override
  public int hashCode() {
    return new HashCodeBuilder()
        .append(section)
        .append(optionSet)
        .toHashCode();
  }

  @Override
  public String toString() {
    return new ToStringBuilder(this)
        .append("section", section)
        .append("optionSet", optionSet)
        .toString();
  }
}
