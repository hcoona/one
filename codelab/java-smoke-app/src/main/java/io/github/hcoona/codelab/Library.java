package io.github.hcoona.codelab;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/** The library give an answer to the universe. */
class Library {
  private static final Logger LOG = LoggerFactory.getLogger(Library.class);
  private static final int ANSWER = 42;

  /** Return the answer. */
  public int getAnswer() {
    LOG.info("You want to know the final answer of the universe.");
    return ANSWER;
  }
}
