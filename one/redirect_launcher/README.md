# Redirect Launcher

This is a tool help to redirect STDOUT & STDERR to rolling files with size limitation.

## Getting Started

```bash
redirect_launcher $PWD/stdout.txt $PWD/stderr.txt <your program with args>
```

## TODO list

1. Read configuration file.
2. Add `port.h` for posix functions on Windows.
