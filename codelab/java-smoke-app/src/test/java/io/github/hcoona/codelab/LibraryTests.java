package io.github.hcoona.codelab;

import static org.assertj.core.api.Assertions.assertThat;

import org.junit.Test;

public class LibraryTests {
  @Test
  public void testGetAnswer() {
    Library library = new Library();
    assertThat(library.getAnswer()).isEqualTo(42);
  }
}
