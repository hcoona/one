package io.github.hcoona.directory_manifest;

import com.google.common.util.concurrent.ThreadFactoryBuilder;
import java.io.IOException;
import java.io.InputStream;
import java.nio.file.FileSystem;
import java.nio.file.FileVisitOption;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributeView;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.attribute.FileTime;
import java.time.Instant;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;
import java.util.Arrays;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import org.apache.commons.codec.digest.DigestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

class Controller {
  private static final Logger LOG = LoggerFactory.getLogger(Controller.class);
  private static final Logger OLOG = LoggerFactory.getLogger("Output");
  private static final DateTimeFormatter ISO_8601_FORMATTER =
      DateTimeFormatter.ofPattern("yyyy-MM-dd'T'hh:mm:ss'Z'");

  private final Path dirPath;
  private final ScheduledExecutorService scheduledExecutorService;

  Controller(FileSystem fs, String dir) {
    this.dirPath = fs.getPath(dir);
    if (!Files.isDirectory(dirPath)) {
      throw new IllegalArgumentException("dir " + dir + " is not a directory");
    }

    ThreadFactory tf =
        new ThreadFactoryBuilder()
            .setNameFormat("File hash calculate thread #%d")
            .setDaemon(true)
            .build();
    this.scheduledExecutorService = new LogErrorScheduledThreadPoolExecutor(6, tf);
  }

  public void run() throws InterruptedException {
    computeDirectory(dirPath).join();
    scheduledExecutorService.shutdown();
    if (!scheduledExecutorService.awaitTermination(1, TimeUnit.SECONDS)) {
      LOG.error("Unfinished tasks: " + scheduledExecutorService.shutdownNow());
    }
  }

  private CompletableFuture<String> computePath(Path path) {
    if (Files.isDirectory(path)) {
      return computeDirectory(path);
    } else {
      return computeFile(path);
    }
  }

  @SuppressWarnings("unchecked")
  private CompletableFuture<String> computeDirectory(Path dir) {
    CompletableFuture<String>[] tasks;
    try {
      tasks =
          (CompletableFuture<String>[])
              Files.walk(dir, 1, FileVisitOption.FOLLOW_LINKS)
                  .skip(1)
                  .map(this::computePath)
                  .toArray(CompletableFuture<?>[]::new);
    } catch (IOException e) {
      LOG.error("Failed to walk dir " + dir, e);
      return CompletableFuture.completedFuture("");
    }

    return CompletableFuture.allOf(tasks)
        .thenApplyAsync(
            ignored ->
                new DigestUtils("MD5")
                    .digestAsHex(
                        Arrays.stream(tasks)
                            .map(
                                t -> {
                                  try {
                                    return t.get();
                                  } catch (InterruptedException e) {
                                    LOG.error("Interrupt computation before it finished", e);
                                  } catch (ExecutionException e) {
                                    LOG.error("Execution failed for task " + t, e);
                                  }
                                  return "";
                                })
                            .reduce((s1, s2) -> s1 + "|" + s2)
                            .orElse("")))
        .thenApplyAsync(
            checksum -> {
              BasicFileAttributeView bfav =
                  Files.getFileAttributeView(dir, BasicFileAttributeView.class);
              try {
                BasicFileAttributes attrs = bfav.readAttributes();
                OLOG.info(
                    String.join(
                        ",",
                        Arrays.asList(
                            "\"" + dir.toString() + "\"",
                            "DIRECTORY",
                            formatFileTime(attrs.creationTime()),
                            formatFileTime(attrs.lastModifiedTime()),
                            String.valueOf(attrs.size()),
                            checksum)));
              } catch (IOException e) {
                LOG.error("Failed to get file attributes " + dir, e);
              }
              return checksum;
            },
            scheduledExecutorService);
  }

  private CompletableFuture<String> computeFile(Path file) {
    return CompletableFuture.supplyAsync(
            () -> {
              try (InputStream is = Files.newInputStream(file, StandardOpenOption.READ)) {
                return new DigestUtils("MD5").digestAsHex(is);
              } catch (IOException e) {
                LOG.error("Failed to compute file " + file);
                return "";
              }
            },
            scheduledExecutorService)
        .thenApplyAsync(
            checksum -> {
              BasicFileAttributeView bfav =
                  Files.getFileAttributeView(file, BasicFileAttributeView.class);
              try {
                BasicFileAttributes attrs = bfav.readAttributes();
                OLOG.info(
                    String.join(
                        ",",
                        Arrays.asList(
                            "\"" + file.toString() + "\"",
                            "FILE",
                            formatFileTime(attrs.creationTime()),
                            formatFileTime(attrs.lastModifiedTime()),
                            String.valueOf(attrs.size()),
                            checksum)));
              } catch (IOException e) {
                LOG.error("Failed to get file attributes " + file, e);
              }
              return checksum;
            },
            scheduledExecutorService);
  }

  private static String formatFileTime(FileTime fileTime) {
    long cTime = fileTime.toMillis();
    ZonedDateTime t = Instant.ofEpochMilli(cTime).atZone(ZoneId.of("UTC"));
    return ISO_8601_FORMATTER.format(t);
  }
}
