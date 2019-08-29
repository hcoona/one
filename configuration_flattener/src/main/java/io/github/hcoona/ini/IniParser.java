package io.github.hcoona.ini;

import java.io.Closeable;
import java.util.ArrayDeque;
import java.util.Iterator;
import java.util.Queue;

public class IniParser implements AutoCloseable {
  private final Iterator<String> lineIterator;
  private final Queue<Event> eventQueue = new ArrayDeque<>();
  private boolean finished = false;
  private String currentLine;
  private boolean inSection = false;

  public IniParser(Iterator<String> lineIterator) {
    this.lineIterator = lineIterator;
  }

  public boolean hasNext() {
    return !finished;
  }

  public Event next() {
    if (!eventQueue.isEmpty()) {
      return eventQueue.poll();
    }

    if (lineIterator.hasNext()) {
      currentLine = lineIterator.next();
    } else {
      finished = true;
      return Event.END_SECTION;
    }

    currentLine = currentLine.trim();
    if (currentLine.isEmpty()) {
      return next();
    }

    if (currentLine.startsWith(";")) {
      return Event.VISIT_COMMENT;
    } else if (currentLine.startsWith("[")) {
      if (inSection) {
        eventQueue.add(Event.END_SECTION);
      }
      eventQueue.add(Event.START_SECTION);
      eventQueue.add(Event.VISIT_SECTION_HEADER);
      inSection = true;
      return eventQueue.poll();
    } else {
      eventQueue.add(Event.VISIT_KEY);
      eventQueue.add(Event.VISIT_VALUE);
      eventQueue.add(Event.END_OPTION);
      return Event.START_OPTION;
    }
  }

  public String getSectionHeader() {
    assert (currentLine.charAt(0) == '[');
    assert (currentLine.charAt(currentLine.length() - 1) == ']');

    return currentLine.substring(1, currentLine.length() - 1);
  }

  public String getOptionKey() {
    int separatorPosition = currentLine.indexOf('=');
    assert (separatorPosition != -1);

    return currentLine.substring(0, separatorPosition);
  }

  public String getOptionValue() {
    int separatorPosition = currentLine.indexOf('=');
    assert (separatorPosition != -1);

    if (separatorPosition == currentLine.length() - 1) {
      return "";
    } else {
      return currentLine.substring(separatorPosition + 1);
    }
  }

  @Override
  public void close() throws Exception {
    if (lineIterator instanceof Closeable) {
      ((Closeable) lineIterator).close();
    }
  }

  public enum Event {
    START_SECTION, END_SECTION,
    START_OPTION, END_OPTION,
    VISIT_SECTION_HEADER,
    VISIT_KEY, VISIT_VALUE,
    VISIT_COMMENT
  }
}
