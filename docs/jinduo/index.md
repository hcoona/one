# jinduo（金铎）

This is a port of muduo（木铎） library.

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

And there are some idea not being decided yet:

* Support io_uring.
* Support coroutine. This must be done very carefully because we have little support debugging with coroutines.
* Provide a generic dispatcher model. This dispatcher could use separately in other systems, such as between stages in [SEDA architecture](http://sosp.org/2001/papers/welsh.pdf), or in Actor pattern, etc.
