package io.github.hcoona.codelab;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

final class Program {
  private static final Logger LOG = LoggerFactory.getLogger(Program.class);

  private Program() {
  }

  public static void main(final String[] args) {
    Library library = new Library();
    LOG.info("The answer is: " + library.getAnswer());
  }
}
