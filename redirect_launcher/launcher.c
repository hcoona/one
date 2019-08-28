#include "redirect_launcher/launcher.h"

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 200809L

#ifdef __STDC_LIB_EXT1__
#define __STDC_WANT_LIB_EXT1__ 1
#else  // __STDC_LIB_EXT1__
#define fprintf_s fprintf
#define snprintf_s snprintf
#endif  // __STDC_LIB_EXT1__

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static const ssize_t kDefaultBufferSize = 4096;
static pthread_once_t init_buffer_size_once = PTHREAD_ONCE_INIT;
static ssize_t buffer_size;

static void InitBufferSize(void) {
  buffer_size = sysconf(_SC_PAGESIZE);
  if (buffer_size < 0) {
    fprintf_s(stderr, "Failed to get system page size: %s\n", strerror(errno));
    buffer_size = kDefaultBufferSize;
  }
}

static const size_t kPathMaxLength = _POSIX_PATH_MAX;

static const int64_t kOutputFileSizeThreshold = 1 << 20;  // 1 MiB
static const int kMaxBackupCount = 3;

static const int kStdoutIndex = 0;
static const int kStderrIndex = 1;
static const int kMaxPipeCount = 1;

static const int kReadPipeIndex = 0;
static const int kWritePipeIndex = 1;

static int stdout_read_fd(int pipes[kMaxPipeCount][2]);
static int stdout_write_fd(int pipes[kMaxPipeCount][2]);
static int stderr_read_fd(int pipes[kMaxPipeCount][2]);
static int stderr_write_fd(int pipes[kMaxPipeCount][2]);

struct ThreadContext {
  const char* output_file;
  int readable_pipe_fd;
  pthread_mutex_t* exit_mutex;
  bool* exiting;              // Guarded by exit_mutex;
  pthread_cond_t* exit_cond;  // Guarded by exit_mutex;
};

static void* CopyPipeToFile(void* context);
static int RollOutputFile(const char* filename);
static _Noreturn void FatalExit();

int launch(const char* stdout_file, const char* stderr_file,
           const char* main_file, char* const argv[]) {
  int ec;
  int exit_code = 0;

  ec = pthread_once(&init_buffer_size_once, InitBufferSize);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to init buffer size: %s\n", strerror(errno));
    return 2;
  }

  int pipes[2][2];
  ec = pipe(pipes[kStdoutIndex]);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to create pipe: %s\n", strerror(errno));
    return 2;
  }
  ec = pipe(pipes[kStderrIndex]);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to create pipe: %s\n", strerror(errno));
    return 2;
  }

  pid_t pid = fork();
  if (pid == -1) {
    fprintf_s(stderr, "Failed to fork child process: %s\n", strerror(errno));
    return 4;
  }

  if (pid == 0) {  // Child process
    ec = dup2(stdout_write_fd(pipes),
              STDOUT_FILENO);  // Redirect STDOUT to our pipe.
    if (ec == -1) {
      fprintf_s(stderr, "Failed to redirect STDOUT to pipe: %s\n",
                strerror(errno));
      return 20;
    }
    ec = dup2(stderr_write_fd(pipes),
              STDERR_FILENO);  // Redirect STDERR to our pipe.
    if (ec == -1) {
      fprintf_s(stderr, "Failed to redirect STDERR to pipe: %s\n",
                strerror(errno));
      return 20;
    }

    // Close before exec for hiding.
    close(stdout_read_fd(pipes));
    close(stdout_write_fd(pipes));
    close(stderr_read_fd(pipes));
    close(stderr_write_fd(pipes));

    return execvp(main_file, argv);
  }

  pthread_mutex_t exit_mutex;
  ec = pthread_mutex_init(&exit_mutex, NULL /* attr */);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to create exit mutex: %s\n", strerror(errno));
    FatalExit();
  }

  bool exiting = false;
  pthread_cond_t exit_cond;
  ec = pthread_cond_init(&exit_cond, NULL /* attr */);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to create exit condition: %s\n", strerror(errno));
    FatalExit();
  }

  struct ThreadContext stdout_context = {
      .output_file = stdout_file,
      .readable_pipe_fd = stdout_read_fd(pipes),
      .exit_mutex = &exit_mutex,
      .exiting = &exiting,
      .exit_cond = &exit_cond,
  };
  struct ThreadContext stderr_context = {
      .output_file = stderr_file,
      .readable_pipe_fd = stderr_read_fd(pipes),
      .exit_mutex = &exit_mutex,
      .exiting = &exiting,
      .exit_cond = &exit_cond,
  };
  pthread_t stdout_tid = 0;
  pthread_t stderr_tid = 0;
  ec = pthread_create(&stdout_tid, NULL /* attr */, &CopyPipeToFile,
                      &stdout_context);
  if (ec != 0) {
    // TODO(zhangshuai.ds): Use strerror_r()
    fprintf_s(stderr, "Failed to create STDOUT redirecting thread: %s\n",
              strerror(errno));
    FatalExit();
  }
  ec = pthread_create(&stderr_tid, NULL /* attr */, &CopyPipeToFile,
                      &stderr_context);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to create STDERR redirecting thread: %s\n",
              strerror(errno));
    FatalExit();
  }

  int status;
  if (waitpid(pid, &status, 0) < 0) {
    fprintf_s(stderr, "Failed to wait child process: %s\n", strerror(errno));
    FatalExit();
  }

  if (WIFEXITED(status)) {
    exit_code = WEXITSTATUS(status);
    // TODO(zhangshuai.ds): Use macro to eliminate debug log when releasing.
    fprintf_s(stdout, "Child process finished with exit code: %d\n", exit_code);
  } else {
    fprintf_s(stderr, "Child process finished abnormally\n");
    exit_code = 255;
  }

  fprintf_s(stdout, "Notify children threads for exiting.\n");

  ec = pthread_mutex_lock(&exit_mutex);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to lock exit mutex: %s\n", strerror(errno));
    return exit_code;
  }

  exiting = true;

  ec = pthread_mutex_unlock(&exit_mutex);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to unlock exit mutex: %s\n", strerror(errno));
    return exit_code;
  }

  ec = pthread_cond_broadcast(&exit_cond);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to unblock redirecting threads: %s\n",
              strerror(errno));
    // Skip joining children threads.
    return exit_code;
  }

  if (stdout_tid != 0) {
    ec = pthread_join(stdout_tid, NULL);
    if (ec != 0) {
      fprintf_s(stderr, "Failed to join STDOUT redirecting thread: %s\n",
                strerror(errno));
    }
  }
  if (stderr_tid != 0) {
    ec = pthread_join(stderr_tid, NULL);
    if (ec != 0) {
      fprintf_s(stderr, "Failed to join STDERR redirecting thread: %s\n",
                strerror(errno));
    }
  }

  return exit_code;
}

int stdout_read_fd(int pipes[kMaxPipeCount][2]) {
  return pipes[kStdoutIndex][kReadPipeIndex];
}
int stdout_write_fd(int pipes[kMaxPipeCount][2]) {
  return pipes[kStdoutIndex][kWritePipeIndex];
}

int stderr_read_fd(int pipes[kMaxPipeCount][2]) {
  return pipes[kStderrIndex][kReadPipeIndex];
}
int stderr_write_fd(int pipes[kMaxPipeCount][2]) {
  return pipes[kStderrIndex][kWritePipeIndex];
}

void* CopyPipeToFile(void* context) {
  struct ThreadContext* the_context = (struct ThreadContext*)context;
  int ec = 0;

  int output_fileno =
      open(the_context->output_file,
           O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NONBLOCK,
           S_IRWXU | S_IRWXG | S_IRWXO);
  if (output_fileno == -1) {
    fprintf_s(stderr, "Failed to open file %s: %s\n", the_context->output_file,
              strerror(errno));
    FatalExit();
  }
  loff_t output_file_offset = 0;

  // Need one more round for remaining contents after received notification.
  bool should_exit = false;

  while (true) {
    while (true) {
      if (output_file_offset > kOutputFileSizeThreshold) {
        ec = close(output_fileno);
        if (ec != 0) {
          fprintf_s(stderr, "Failed to close %s: %s\n",
                    the_context->output_file, strerror(errno));
        }

        ec = RollOutputFile(the_context->output_file);
        if (ec != 0) {
          fprintf_s(stderr, "Failed to roll %s: %s\n", the_context->output_file,
                    strerror(errno));
        }

        output_fileno =
            open(the_context->output_file,
                 O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NONBLOCK,
                 S_IRWXU | S_IRWXG | S_IRWXO);
        if (output_fileno == -1) {
          fprintf_s(stderr,
                    "Failed to open file %s: %s; Fallback to /dev/null.\n",
                    the_context->output_file, strerror(errno));
          output_fileno = open("/dev/null", O_WRONLY | O_NONBLOCK);
          if (output_fileno == -1) {
            fprintf_s(stderr, "Failed to open /dev/null: %s\n",
                      strerror(errno));
            FatalExit();
          }
        }
        output_file_offset = 0;
        fprintf_s(stdout, "Open new file %s for rolling\n",
                  the_context->output_file);
      }

      ssize_t count = splice(
          the_context->readable_pipe_fd, NULL /* offset_in */, output_fileno,
          &output_file_offset, buffer_size, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
      if (count == -1) {
        if (errno == EAGAIN) {
          break;
        }
        fprintf_s(stderr, "Failed to splice contents from pipe %d to %s: %s\n",
                  the_context->readable_pipe_fd, the_context->output_file,
                  strerror(errno));
        break;
      }
      if (count == 0) {
        break;
      }
      fprintf_s(stdout, "%zd bytes transferred from pipe %d to %s\n", count,
                the_context->readable_pipe_fd, the_context->output_file);
    }

    if (should_exit) {
      break;
    }

    ec = pthread_mutex_lock(the_context->exit_mutex);
    if (ec != 0) {
      fprintf_s(stderr, "Failed to lock exit mutex: %s\n", strerror(errno));
      FatalExit();
    }

    struct timeval now;
    ec = gettimeofday(&now, NULL /* timezone */);
    if (ec != 0) {
      fprintf_s(stderr, "Failed to get current time: %s\n", strerror(errno));
      FatalExit();
    }

    fprintf_s(stdout, "Wait 1 second for next round or exiting.\n");

    // TODO(zhangshuai.ds): Load from settings
    struct timespec timeout = {.tv_sec = now.tv_sec + 1,
                               .tv_nsec = now.tv_usec * 1000};
    ec = pthread_cond_timedwait(the_context->exit_cond, the_context->exit_mutex,
                                &timeout);
    if (ec != 0 && ec != ETIMEDOUT) {
      fprintf_s(stderr, "Failed to wait exit condition: %s\n", strerror(errno));
      FatalExit();
    }

    should_exit = *(the_context->exiting);

    int ec_unlock = pthread_mutex_unlock(the_context->exit_mutex);
    if (ec_unlock != 0) {
      fprintf_s(stderr, "Failed to unlock exit mutex: %s\n", strerror(errno));
      FatalExit();
    }
  }

  ec = close(output_fileno);
  if (ec != 0) {
    fprintf_s(stderr, "Failed to close %s: %s\n", the_context->output_file,
              strerror(errno));
  }

  return NULL;
}

int RollOutputFile(const char* filename) {
  int ec = 0;
  char path_buffer[2][kPathMaxLength];
  int current_index = 0;
  int previous_index = 1;

  snprintf_s(path_buffer[previous_index], sizeof(path_buffer), "%s.%d",
             filename, kMaxBackupCount);
  for (int i = kMaxBackupCount - 1; i >= 0; i--) {
    if (i == 0) {
      snprintf_s(path_buffer[current_index], sizeof(path_buffer), "%s",
                 filename);
    } else {
      snprintf_s(path_buffer[current_index], sizeof(path_buffer), "%s.%d",
                 filename, i);
    }

    ec = access(path_buffer[current_index], F_OK);
    if (ec == 0) {
      fprintf_s(stdout, "Renaming from %s to %s\n", path_buffer[current_index],
                path_buffer[previous_index]);
      ec = rename(path_buffer[current_index], path_buffer[previous_index]);
      if (ec != 0) {
        fprintf_s(stderr, "Failed to rename file from %s to %s: %s\n",
                  path_buffer[current_index], path_buffer[previous_index],
                  strerror(errno));
        return ec;
      }
    }

    // Swap current_index & previous_index.
    current_index ^= previous_index;
    previous_index ^= current_index;
    current_index ^= previous_index;
  }

  return 0;
}

_Noreturn void FatalExit() {
  if (kill(-getpgrp(), SIGKILL) != 0) {
    fprintf_s(stderr, "Failed to kill child process group: %s\n",
              strerror(errno));
  }
  abort();
}
