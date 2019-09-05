package io.github.hcoona;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.IOException;

public class Converter {
  private static Logger LOG = LoggerFactory.getLogger(Converter.class);

  public void parseConfigStatusFile(BufferedReader bufferedReader) throws IOException {
    while (true) {
      String line = bufferedReader.readLine();
      if (line == null) {
        break;
      }

      if (line.startsWith("S[\"")) {
        LOG.info(line);
      }
    }
  }

  public void parseMakefileAm(BufferedReader bufferedReader) throws IOException {
    while (true) {
      String line = bufferedReader.readLine();
      if (line == null) {
        break;
      }

      if (line.startsWith("if ")) { // Begin condition.
        LOG.info(line);
      }
    }
  }
}
