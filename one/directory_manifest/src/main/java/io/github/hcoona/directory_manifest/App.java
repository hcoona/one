package io.github.hcoona.directory_manifest;

import java.nio.file.FileSystems;
import java.nio.file.Paths;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class App {
  private static final Logger LOG = LoggerFactory.getLogger(App.class);

  public static void main(String[] args) {
    String dir = args.length == 0 ? Paths.get("").toAbsolutePath().toString() : args[0];
    Controller controller = new Controller(FileSystems.getDefault(), dir);

    try {
      controller.run();
    } catch (InterruptedException e) {
      LOG.error("Interrupted before it finished", e);
    }
  }
}
