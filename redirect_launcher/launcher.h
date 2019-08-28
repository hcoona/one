#ifndef REDIRECT_LAUNCHER_LAUNCHER_H_
#define REDIRECT_LAUNCHER_LAUNCHER_H_

#ifdef __cplusplus
extern "C" {
#endif

int launch(const char* stdout_file, const char* stderr_file,
           const char* main_file, char* const argv[]);

#ifdef __cplusplus
}
#endif

#endif  // REDIRECT_LAUNCHER_LAUNCHER_H_
