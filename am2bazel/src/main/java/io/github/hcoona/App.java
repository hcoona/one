package io.github.hcoona;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Objects;

import static java.nio.charset.StandardCharsets.UTF_8;

public final class App {
  private App() {}

  private static Logger LOG = LoggerFactory.getLogger(App.class);

  public static void main(String[] args) throws IOException {
    Converter converter = new Converter();

    try (InputStream inputStream =
            Objects.requireNonNull(
                App.class.getClassLoader().getResourceAsStream("config.status"));
        InputStreamReader inputStreamReader = new InputStreamReader(inputStream, UTF_8);
        BufferedReader bufferedReader = new BufferedReader(inputStreamReader)) {
      converter.parseConfigStatusFile(bufferedReader);
    }

    try (InputStream inputStream =
            Objects.requireNonNull(App.class.getClassLoader().getResourceAsStream("Makefile.am"));
        InputStreamReader inputStreamReader = new InputStreamReader(inputStream, UTF_8);
        BufferedReader bufferedReader = new BufferedReader(inputStreamReader)) {
      converter.parseMakefileAm(bufferedReader);
    }
  }
}
