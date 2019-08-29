package io.github.hcoona.ini;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.HashMap;
import java.util.HashSet;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

public class IniReaderTest {
  private static final ClassLoader CLASS_LOADER = IniReaderTest.class.getClassLoader();

  @Test
  void testReaderCase1() {
    final IniFileObject<String, String, String> expected = getExpectedForCase1();
    final IniFileObject<String, String, String> actual;
    try (InputStream is = CLASS_LOADER.getResourceAsStream("ini/test_case1.ini")) {
      try (BufferedReader reader = new BufferedReader(new InputStreamReader(is))) {
        IniParser parser = new IniParser(reader.lines().iterator());
        IniReader<String, String, String> iniReader =
            new IniReader<>(
                parser,
                HashMap::new,
                HashSet::new,
                Function.identity(),
                Function.identity(),
                Function.identity());
        actual = iniReader.read();
        Assertions.assertEquals(expected, actual);
      }
    } catch (IOException e) {
      Assertions.fail(e);
    }
  }

  private IniFileObject<String, String, String> getExpectedForCase1() {
    IniFileObject<String, String, String> result = new IniFileObject<>(new HashMap<>());
    result.put(
        new IniSectionObject<>(
            "header1",
            Stream.of(new IniOptionObject<>("X", "a"), new IniOptionObject<>("Y", "b"))
                .collect(Collectors.toSet())));
    result.put(
        new IniSectionObject<>(
            "header2",
            Stream.of(new IniOptionObject<>("Z", "c"), new IniOptionObject<>("U", "d"))
                .collect(Collectors.toSet())));
    result.put(
        new IniSectionObject<>(
            "header3",
            Stream.of(new IniOptionObject<>("V", "e,f,g"), new IniOptionObject<>("W", "h;i;j"))
                .collect(Collectors.toSet())));
    return result;
  }
}
