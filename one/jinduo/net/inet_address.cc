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

#include "one/jinduo/net/inet_address.h"

#include <netdb.h>
#include <netinet/in.h>

#include "absl/base/internal/endian.h"
#include "glog/logging.h"
#include "one/jinduo/net/internal/sockets_ops.h"

namespace jinduo {
namespace net {

namespace {

// INADDR_ANY use (type)value casting.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
constexpr in_addr_t kInaddrAny = INADDR_ANY;
constexpr in_addr_t kInaddrLoopback = INADDR_LOOPBACK;
#pragma GCC diagnostic pop

constexpr size_t kAddressStringSize = 64;
constexpr size_t kHostnameItemCountMax = 1024;

}  // namespace

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6),
              "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset 2");

InetAddress::InetAddress(uint16_t portArg, bool loopbackOnly, bool ipv6) {
  static_assert(offsetof(InetAddress, data_.addr6) == 0,
                "data_.addr6 offset 0");
  static_assert(offsetof(InetAddress, data_.addr) == 0, "data_.addr offset 0");
  if (ipv6) {
    ::memset(&data_.addr6, 0, sizeof data_.addr6);
    data_.addr6.sin6_family = AF_INET6;
    in6_addr ip = loopbackOnly ? in6addr_loopback : in6addr_any;
    data_.addr6.sin6_addr = ip;
    data_.addr6.sin6_port = absl::big_endian::FromHost16(portArg);
  } else {
    ::memset(&data_.addr, 0, sizeof data_.addr);
    data_.addr.sin_family = AF_INET;
    in_addr_t ip = loopbackOnly ? kInaddrLoopback : kInaddrAny;
    data_.addr.sin_addr.s_addr = absl::big_endian::FromHost32(ip);
    data_.addr.sin_port = absl::big_endian::FromHost16(portArg);
  }
}

InetAddress::InetAddress(hcoona::CStringArg ip, uint16_t portArg, bool ipv6) {
  if (ipv6 || (::strchr(ip.c_str(), ':') != nullptr)) {
    ::memset(&data_.addr6, 0, sizeof data_.addr6);
    sockets::fromIpPort(ip.c_str(), portArg, &data_.addr6);
  } else {
    ::memset(&data_.addr, 0, sizeof data_.addr);
    sockets::fromIpPort(ip.c_str(), portArg, &data_.addr);
  }
}

std::string InetAddress::toIpPort() const {
  char buf[kAddressStringSize] = "";
  sockets::toIpPort(buf, sizeof buf, getSockAddr());
  return buf;
}

std::string InetAddress::toIp() const {
  char buf[kAddressStringSize] = "";
  sockets::toIp(buf, sizeof buf, getSockAddr());
  return buf;
}

uint32_t InetAddress::ipv4NetEndian() const {
  assert(family() == AF_INET);
  return data_.addr.sin_addr.s_addr;
}

uint16_t InetAddress::port() const {
  return absl::big_endian::ToHost16(portNetEndian());
}

static __thread char
    t_resolveBuffer[kAddressStringSize * kHostnameItemCountMax];

bool InetAddress::resolve(hcoona::CStringArg hostname, InetAddress* result) {
  assert(result != nullptr);
  hostent hent{};
  ::memset(&hent, 0, sizeof(hent));

  hostent* he = nullptr;
  int herrno = 0;

  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer,
                            sizeof t_resolveBuffer, &he, &herrno);
  if (ret == 0 && he != nullptr) {
    assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
    result->data_.addr.sin_addr =
        *reinterpret_cast<struct in_addr*>(he->h_addr);
    return true;
  }
  if (ret != 0) {
    LOG(ERROR) << "InetAddress::resolve";
  }
  return false;
}

void InetAddress::setScopeId(uint32_t scope_id) {
  if (family() == AF_INET6) {
    data_.addr6.sin6_scope_id = scope_id;
  }
}

}  // namespace net
}  // namespace jinduo
