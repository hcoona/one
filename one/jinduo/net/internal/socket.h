// Copyright (c) 2022 Zhang Shuai<zhangshuai.ustc@gmail.com>.
// All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#pragma once

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace jinduo {
namespace net {

class InetAddress;

// Wrapper of socket file descriptor.
//
// It closes the sockfd when desctructs.
// It's thread safe, all operations are delagated to OS.
class Socket {
 public:
  explicit Socket(int sockfd) : sockfd_(sockfd) {}

  // Socket(Socket&&) // move constructor in C++11
  ~Socket();

  // Disallow copy.
  Socket(const Socket&) noexcept = delete;
  Socket& operator=(const Socket&) noexcept = delete;

  // Allow move but not implemented yet.
  Socket(Socket&&) noexcept = delete;
  Socket& operator=(Socket&&) noexcept = delete;

  [[nodiscard]] int fd() const { return sockfd_; }
  // return true if success.
  bool getTcpInfo(struct tcp_info*) const;
  bool getTcpInfoString(char* buf, int len) const;

  // abort if address in use
  void bindAddress(const InetAddress& localaddr);
  // abort if address in use
  void listen();

  // On success, returns a non-negative integer that is
  // a descriptor for the accepted socket, which has been
  // set to non-blocking and close-on-exec. *peeraddr is assigned.
  // On error, -1 is returned, and *peeraddr is untouched.
  int accept(InetAddress* peeraddr);

  void shutdownWrite();

  // Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
  void setTcpNoDelay(bool on);

  // Enable/disable SO_REUSEADDR
  void setReuseAddr(bool on);

  // Enable/disable SO_REUSEPORT
  void setReusePort(bool on);

  // Enable/disable SO_KEEPALIVE
  void setKeepAlive(bool on);

 private:
  const int sockfd_;
};

}  // namespace net
}  // namespace jinduo
