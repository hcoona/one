#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int launch(int size_mib, int backup_count, const char* stdout_file,
           const char* stderr_file, const char* main_file, char* const argv[]);

#ifdef __cplusplus
}
#endif
