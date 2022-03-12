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
// This is a public header file, it must only include public header files.

#pragma once

#include <netinet/in.h>

#include <string>

#include "one/base/c_string_arg.h"

namespace jinduo {
namespace net {
namespace sockets {

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);

}  // namespace sockets

// Wrapper of sockaddr_in.
//
// This is an POD interface class.
class InetAddress {
 public:
  /// Constructs an endpoint with given port number.
  /// Mostly used in TcpServer listening.
  explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false,
                       bool ipv6 = false);

  /// Constructs an endpoint with given ip and port.
  /// @c ip should be "1.2.3.4"
  InetAddress(hcoona::CStringArg ip, uint16_t port, bool ipv6 = false);

  /// Constructs an endpoint with given struct @c sockaddr_in
  /// Mostly used when accepting new connections
  explicit InetAddress(const struct sockaddr_in& addr) : data_(addr) {}

  explicit InetAddress(const struct sockaddr_in6& addr) : data_(addr) {}

  [[nodiscard]] sa_family_t family() const { return data_.addr.sin_family; }
  [[nodiscard]] std::string toIp() const;
  [[nodiscard]] std::string toIpPort() const;
  [[nodiscard]] uint16_t port() const;

  // default copy/assignment are Okay

  [[nodiscard]] const struct sockaddr* getSockAddr() const {
    return sockets::sockaddr_cast(&data_.addr6);
  }
  void setSockAddrInet6(const struct sockaddr_in6& addr6) {
    data_.addr6 = addr6;
  }

  [[nodiscard]] uint32_t ipv4NetEndian() const;
  [[nodiscard]] uint16_t portNetEndian() const { return data_.addr.sin_port; }

  // resolve hostname to IP address, not changing port or sin_family
  // return true on success.
  // thread safe
  static bool resolve(hcoona::CStringArg hostname, InetAddress* result);
  // static std::vector<InetAddress> resolveAll(const char* hostname, uint16_t
  // port = 0);

  // set IPv6 ScopeID
  void setScopeId(uint32_t scope_id);

 private:
  union Data {
    Data() = default;
    explicit Data(const struct sockaddr_in& addr) : addr(addr) {}
    explicit Data(const struct sockaddr_in6& addr6) : addr6(addr6) {}

    struct sockaddr_in addr;
    struct sockaddr_in6 addr6;
  } data_{};
};

}  // namespace net
}  // namespace jinduo
