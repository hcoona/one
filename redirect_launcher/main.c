#include <stdio.h>
#include <stdlib.h>
#include "redirect_launcher/launcher.h"

int main(int argc, char** argv) {
  if (argc < 6) {
    fprintf(stderr,
            "Usage: %s <int32 size_mib> <int32 backup_count> <stdout_file> "
            "<stderr_file> <main_file> [args]\n",
            argv[0]);
    return 1;
  }

  int size_mib = atoi(argv[1]);
  if (size_mib <= 0) {
    fprintf(stderr, "Invalid input parameter size_mib: %s\n", argv[1]);
    return 1;
  }

  int backup_count = atoi(argv[2]);
  if (backup_count <= 0) {
    fprintf(stderr, "Invalid input parameter backup_count: %s\n", argv[1]);
    return 1;
  }

  return launch(size_mib, backup_count, argv[3], argv[4], argv[5], &argv[5]);
}
