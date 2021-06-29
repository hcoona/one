#include "one/redirect_launcher/launcher.h"

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

#define LAUNCHER_LOGF_ERROR(s_, ...) \
  fprintf_s(stderr, "[ERROR] " s_, __VA_ARGS__)

#define LAUNCHER_LOG_WARN(s_, ...) fprintf_s(stderr, "[WARN] " s_)
#define LAUNCHER_LOGF_WARN(s_, ...) fprintf_s(stderr, "[WARN] " s_, __VA_ARGS__)

#define LAUNCHER_LOG_INFO(s_, ...) fprintf_s(stdout, "[INFO] " s_)
#define LAUNCHER_LOGF_INFO(s_, ...) fprintf_s(stdout, "[INFO] " s_, __VA_ARGS__)

#ifdef NDEBUG
#define LAUNCHER_DLOG(s_, ...)
#define LAUNCHER_DLOGF(s_, ...)
#else  // NDEBUG
#define LAUNCHER_DLOG(s_) fprintf_s(stdout, "[DEBUG] " s_)
#define LAUNCHER_DLOGF(s_, ...) fprintf_s(stdout, "[DEBUG] " s_, __VA_ARGS__)
#endif  // NDEBUG

static const int64_t kOneMillisecondInNanoseconds = 1000;
static const int64_t kOneSecondInNanoseconds = 1E9L;

static const int kLoopIntervalMilliseconds = 10;
static const int64_t kLoopIntervalNanoseconds =
    kLoopIntervalMilliseconds * kOneMillisecondInNanoseconds;

static const ssize_t kDefaultBufferSize = 4096;
static pthread_once_t init_buffer_size_once = PTHREAD_ONCE_INIT;
static ssize_t buffer_size;

static void InitBufferSize(void) {
  buffer_size = sysconf(_SC_PAGESIZE);
  if (buffer_size < 0) {
    LAUNCHER_LOGF_WARN("Failed to get system page size: %s\n", strerror(errno));
    buffer_size = kDefaultBufferSize;
  }
}

static const size_t kPathMaxLength = _POSIX_PATH_MAX;

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

  int64_t output_file_size_threshold_bytes;
  int max_backup_count;

  pthread_mutex_t* exit_mutex;
  bool* exiting;              // Guarded by exit_mutex;
  pthread_cond_t* exit_cond;  // Guarded by exit_mutex;
};

static void* CopyPipeToFile(void* context);
static int MoveFirstOutputFile(const char* filename);
static int RollOutputFile(const char* filename, int backup_count);
static _Noreturn void FatalExit();

int launch(int size_mib, int backup_count, const char* stdout_file,
           const char* stderr_file, const char* main_file, char* const argv[]) {
  int ec;
  int exit_code = 0;

  ec = pthread_once(&init_buffer_size_once, InitBufferSize);
  if (ec != 0) {
    LAUNCHER_LOGF_WARN("Failed to init buffer size: %s\n", strerror(errno));
    return 2;
  }

  int pipes[2][2];
  ec = pipe(pipes[kStdoutIndex]);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to create pipe: %s\n", strerror(errno));
    return 2;
  }
  ec = pipe(pipes[kStderrIndex]);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to create pipe: %s\n", strerror(errno));
    return 2;
  }

  pid_t pid = fork();
  if (pid == -1) {
    LAUNCHER_LOGF_ERROR("Failed to fork child process: %s\n", strerror(errno));
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
      LAUNCHER_LOGF_ERROR("Failed to redirect STDERR to pipe: %s\n",
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
    LAUNCHER_LOGF_ERROR("Failed to create exit mutex: %s\n", strerror(errno));
    FatalExit();
  }

  bool exiting = false;
  pthread_cond_t exit_cond;
  ec = pthread_cond_init(&exit_cond, NULL /* attr */);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to create exit condition: %s\n",
                        strerror(errno));
    FatalExit();
  }

  struct ThreadContext stdout_context = {
      .output_file = stdout_file,
      .readable_pipe_fd = stdout_read_fd(pipes),
      .output_file_size_threshold_bytes = size_mib << 20,
      .max_backup_count = backup_count,
      .exit_mutex = &exit_mutex,
      .exiting = &exiting,
      .exit_cond = &exit_cond,
  };
  struct ThreadContext stderr_context = {
      .output_file = stderr_file,
      .readable_pipe_fd = stderr_read_fd(pipes),
      .output_file_size_threshold_bytes = size_mib << 20,
      .max_backup_count = backup_count,
      .exit_mutex = &exit_mutex,
      .exiting = &exiting,
      .exit_cond = &exit_cond,
  };
  pthread_t stdout_tid = 0;
  pthread_t stderr_tid = 0;
  ec = pthread_create(&stdout_tid, NULL /* attr */, &CopyPipeToFile,
                      &stdout_context);
  if (ec != 0) {
    // TODO(zhangshuai.ustc): Use strerror_r()
    LAUNCHER_LOGF_ERROR("Failed to create STDOUT redirecting thread: %s\n",
                        strerror(errno));
    FatalExit();
  }
  ec = pthread_create(&stderr_tid, NULL /* attr */, &CopyPipeToFile,
                      &stderr_context);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to create STDERR redirecting thread: %s\n",
                        strerror(errno));
    FatalExit();
  }

  int status;
  if (waitpid(pid, &status, 0) < 0) {
    LAUNCHER_LOGF_ERROR("Failed to wait child process: %s\n", strerror(errno));
    FatalExit();
  }

  if (WIFEXITED(status)) {
    exit_code = WEXITSTATUS(status);
    LAUNCHER_LOGF_INFO("Child process finished with exit code: %d\n",
                       exit_code);
  } else {
    LAUNCHER_LOG_WARN("Child process finished abnormally\n");
    exit_code = 255;
  }

  LAUNCHER_LOG_INFO("Notify children threads for exiting.\n");

  ec = pthread_mutex_lock(&exit_mutex);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to lock exit mutex: %s\n", strerror(errno));
    return exit_code;
  }

  exiting = true;

  ec = pthread_mutex_unlock(&exit_mutex);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to unlock exit mutex: %s\n", strerror(errno));
    return exit_code;
  }

  ec = pthread_cond_broadcast(&exit_cond);
  if (ec != 0) {
    LAUNCHER_LOGF_ERROR("Failed to unblock redirecting threads: %s\n",
                        strerror(errno));
    // Skip joining children threads.
    return exit_code;
  }

  if (stdout_tid != 0) {
    ec = pthread_join(stdout_tid, NULL);
    if (ec != 0) {
      LAUNCHER_LOGF_WARN("Failed to join STDOUT redirecting thread: %s\n",
                         strerror(errno));
    }
  }
  if (stderr_tid != 0) {
    ec = pthread_join(stderr_tid, NULL);
    if (ec != 0) {
      LAUNCHER_LOGF_WARN("Failed to join STDERR redirecting thread: %s\n",
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

  bool first_rotating = true;
  int output_fileno =
      open(the_context->output_file,
           O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NONBLOCK,
           S_IRWXU | S_IRWXG | S_IRWXO);
  if (output_fileno == -1) {
    LAUNCHER_LOGF_ERROR("Failed to open file %s: %s\n",
                        the_context->output_file, strerror(errno));
    FatalExit();
  }
  loff_t output_file_offset = 0;

  // Need one more round for remaining contents after received notification.
  bool should_exit = false;

  while (true) {
    while (true) {
      if (output_file_offset > the_context->output_file_size_threshold_bytes) {
        LAUNCHER_LOG_INFO("Begin rollover log file.\n");
        ec = close(output_fileno);
        if (ec != 0) {
          LAUNCHER_LOGF_WARN("Failed to close %s: %s\n",
                             the_context->output_file, strerror(errno));
        }

        if (first_rotating) {
          ec = MoveFirstOutputFile(the_context->output_file);
          if (ec != 0) {
            LAUNCHER_LOGF_WARN("Failed to roll %s: %s\n",
                               the_context->output_file, strerror(errno));
          }
          first_rotating = false;
        } else {
          ec = RollOutputFile(the_context->output_file,
                              the_context->max_backup_count);
          if (ec != 0) {
            LAUNCHER_LOGF_WARN("Failed to roll %s: %s\n",
                               the_context->output_file, strerror(errno));
          }
        }

        output_fileno =
            open(the_context->output_file,
                 O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NONBLOCK,
                 S_IRWXU | S_IRWXG | S_IRWXO);
        if (output_fileno == -1) {
          LAUNCHER_LOGF_WARN(
              "Failed to open file %s: %s; Fallback to /dev/null.\n",
              the_context->output_file, strerror(errno));
          output_fileno = open("/dev/null", O_WRONLY | O_NONBLOCK);
          if (output_fileno == -1) {
            LAUNCHER_LOGF_ERROR("Failed to open /dev/null: %s\n",
                                strerror(errno));
            FatalExit();
          }
        }
        output_file_offset = 0;
        LAUNCHER_LOG_INFO("Finish rollover log file.\n");
      }

      ssize_t count = splice(
          the_context->readable_pipe_fd, NULL /* offset_in */, output_fileno,
          &output_file_offset, buffer_size, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);
      if (count == -1) {
        if (errno == EAGAIN) {
          break;
        }
        LAUNCHER_LOGF_WARN("Failed to splice contents from pipe %d to %s: %s\n",
                           the_context->readable_pipe_fd,
                           the_context->output_file, strerror(errno));
        break;
      }
      if (count == 0) {
        break;
      }
      LAUNCHER_DLOGF("%zd bytes transferred from pipe %d to %s\n", count,
                     the_context->readable_pipe_fd, the_context->output_file);
    }

    if (should_exit) {
      break;
    }

    ec = pthread_mutex_lock(the_context->exit_mutex);
    if (ec != 0) {
      LAUNCHER_LOGF_ERROR("Failed to lock exit mutex: %s\n", strerror(errno));
      FatalExit();
    }

    struct timeval now;
    ec = gettimeofday(&now, NULL /* timezone */);
    if (ec != 0) {
      LAUNCHER_LOGF_ERROR("Failed to get current time: %s\n", strerror(errno));
      FatalExit();
    }

    LAUNCHER_DLOGF("Wait %d ms for next round or exiting.\n",
                   kLoopIntervalMilliseconds);

    // TODO(zhangshuai.ustc): Load from settings
    struct timespec deadline;  // deadline = now + interval
    if (now.tv_usec + kLoopIntervalNanoseconds >= kOneSecondInNanoseconds) {
      deadline.tv_sec = now.tv_sec + 1,
      deadline.tv_nsec =
          now.tv_usec + (kLoopIntervalNanoseconds - kOneSecondInNanoseconds);
    } else {
      deadline.tv_sec = now.tv_sec,
      deadline.tv_nsec = now.tv_usec + kLoopIntervalNanoseconds;
    }

    ec = pthread_cond_timedwait(the_context->exit_cond, the_context->exit_mutex,
                                &deadline);
    if (ec != 0 && ec != ETIMEDOUT) {
      LAUNCHER_LOGF_ERROR("Failed to wait exit condition: %s\n",
                          strerror(errno));
      FatalExit();
    }

    should_exit = *(the_context->exiting);

    int ec_unlock = pthread_mutex_unlock(the_context->exit_mutex);
    if (ec_unlock != 0) {
      LAUNCHER_LOGF_ERROR("Failed to unlock exit mutex: %s\n", strerror(errno));
      FatalExit();
    }
  }

  ec = close(output_fileno);
  if (ec != 0) {
    LAUNCHER_LOGF_WARN("Failed to close %s: %s\n", the_context->output_file,
                       strerror(errno));
  }

  return NULL;
}

int MoveFirstOutputFile(const char* filename) {
  int ec = 0;
  char path_buffer[kPathMaxLength];

  snprintf_s(path_buffer, kPathMaxLength, "%s.head", filename);
  LAUNCHER_LOGF_INFO("Renaming from %s to %s\n", filename, path_buffer);
  ec = rename(filename, path_buffer);
  if (ec != 0) {
    LAUNCHER_LOGF_WARN("Failed to rename file from %s to %s: %s\n", filename,
                       path_buffer, strerror(errno));
    return ec;
  }

  return 0;
}

int RollOutputFile(const char* filename, int backup_count) {
  int ec = 0;
  char path_buffer[2][kPathMaxLength];
  int current_index = 0;
  int previous_index = 1;

  snprintf_s(path_buffer[previous_index], kPathMaxLength, "%s.%d", filename,
             backup_count);
  for (int i = backup_count - 1; i >= 0; i--) {
    if (i == 0) {
      snprintf_s(path_buffer[current_index], kPathMaxLength, "%s", filename);
    } else {
      snprintf_s(path_buffer[current_index], kPathMaxLength, "%s.%d", filename,
                 i);
    }

    ec = access(path_buffer[current_index], F_OK);
    if (ec == 0) {
      LAUNCHER_LOGF_INFO("Renaming from %s to %s\n", path_buffer[current_index],
                         path_buffer[previous_index]);
      ec = rename(path_buffer[current_index], path_buffer[previous_index]);
      if (ec != 0) {
        LAUNCHER_LOGF_WARN("Failed to rename file from %s to %s: %s\n",
                           path_buffer[current_index],
                           path_buffer[previous_index], strerror(errno));
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
    LAUNCHER_LOGF_WARN("Failed to kill child process group: %s\n",
                       strerror(errno));
  }
  abort();
}
