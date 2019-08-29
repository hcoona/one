package io.github.hcoona.ini;

import io.github.hcoona.ini.IniParser.Event;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Queue;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

public class IniParserTest {
  private static final ClassLoader CLASS_LOADER = IniParserTest.class.getClassLoader();

  @Test
  void testParseEventCase1() {
    final Queue<Event> events = new ArrayDeque<>();

    try (InputStream is = CLASS_LOADER.getResourceAsStream("ini/test_case1.ini")) {
      try (BufferedReader reader = new BufferedReader(new InputStreamReader(is))) {
        IniParser parser = new IniParser(reader.lines().iterator());
        while (parser.hasNext()) {
          events.add(parser.next());
        }
      }
    } catch (IOException e) {
      Assertions.fail(e);
    }

    Assertions.assertIterableEquals(
        Arrays.asList(Event.VISIT_COMMENT, Event.START_SECTION, Event.VISIT_SECTION_HEADER,
            Event.START_OPTION, Event.VISIT_KEY, Event.VISIT_VALUE, Event.END_OPTION,
            Event.START_OPTION, Event.VISIT_KEY, Event.VISIT_VALUE, Event.END_OPTION,
            Event.END_SECTION, Event.START_SECTION, Event.VISIT_SECTION_HEADER, Event.VISIT_COMMENT,
            Event.START_OPTION, Event.VISIT_KEY, Event.VISIT_VALUE, Event.END_OPTION,
            Event.START_OPTION, Event.VISIT_KEY, Event.VISIT_VALUE, Event.END_OPTION,
            Event.END_SECTION, Event.START_SECTION, Event.VISIT_SECTION_HEADER, Event.START_OPTION,
            Event.VISIT_KEY, Event.VISIT_VALUE, Event.END_OPTION, Event.START_OPTION,
            Event.VISIT_KEY, Event.VISIT_VALUE, Event.END_OPTION, Event.VISIT_COMMENT,
            Event.END_SECTION),
        events);
  }
}
