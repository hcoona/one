# jinduo（金铎）

This is a port of muduo（木铎） library.

## What's muduo（木铎）?

muduo is a high quality & performance network library. It's personally owned by Chen Shuo. The design philosophy of muduo project is a little different with my expectations.

The design philosophy of muduo is recorded in book 《Linux 多线程服务端编程：使用 muduo C++ 网络库》, section 6.1

* Thread-safety, natively support multi-core & multi-threading.
* Non-portable, only support Linux.
* Only support x86-64 arch, support IA64 only if possible.
* Only support TCP.
* Only support IPv4.
* Assume communication happened in a LAN.
* Only support one pattern: non-blocking I/O + one event loop per thread.
* Simplified API. Only expose concrete class & classes defined in STL. Won't use non-trivial templates & virtual methods in API.
* Only consider the most frequently used functionalities.
* Be a library instead of a framework.
* LOC <= 5000 (tests not count in).
* Support FreeBSD/Darwin OS without increasing complexity. AKA. use `poll(2)` & `epoll(4)` for IO multiplexing.
* Integrate with Google Protocol Buffers RPC if possible.

## What's changes in jinduo（金铎）?

I'd like to make following changes to muduo library:

* Style
    * clang-format with Google style.
    * Use `#pragma once` for header guard.
    * Do not use `noncopyable`, use `= delete` instead.
* Basic support
    * Replace the `base` directory with our `base`/STL/abseil-cpp/...
    * Support only C++17 and higher.
* Feature
    * Support IPv6 (muduo already supported?).
    * Support signal event.
    * Redesign the `Buffer` class. The current design has a great chance never shrink the spaces.
* Performance
    * Use C++11 `move` for performance.
* Chore
    * Regular CI pipeline.
    * Testing against sanitizers.

## Future?

There are some idea not being decided yet:

* Support io_uring.
* Support coroutine. This must be done very carefully because we have little support debugging with coroutines.

## Design considerations

### Why not share thread pool when running multiple TCP listeners?

It's necessary to launch multiple TCP listeners with TCP option SO_REUSEPORT for fast TCP accepting. In such scenario, each listeners would backed with a thread pool for TCP connection IO. Should we share the IO thread pool across the TCP listeners?

Fetching a child event loop  from the thread pool, we need to ensure we are running in its parent event loop to avoid thread-safety & performance issues. This requires a relationship between child event loop & parent event loop. We designed to run a single event loop in a single thread. So we cannot share the IO thread pool across the TCP listeners.

Still need further investigation, it seems that the abstraction of Java NIO support such a model.

### Could we use a dispatcher model?

I think dispatcher model is different from our poller model.

We need the thread pull events from a set of fd(s) and read from/write to the active fd. The dispatcher model would pull nothing, instead it just dispatch the items in queue to downstream.

## Learned from Java NIO & Netty

### High resolution epoll wait

Use `epoll_pwait2` if possible (require kernel 5.11+).

Fallback to `epoll_wait` if we allow milliseconds precision.

Use `epoll_wait` & `timerfd` to simulate an high resolution wait.

### TCP & epoll options

* `TCP_FASTOPEN_CONNECT`
* `TCP_FASTOPEN`

* `EPOLLRDHUP`
