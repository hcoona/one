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

#include <string>

#include "gtest/gtest.h"

using jinduo::net::InetAddress;

TEST(InetAddress, IPv4) {
  InetAddress addr0(1234);
  EXPECT_EQ(addr0.toIp(), std::string("0.0.0.0"));
  EXPECT_EQ(addr0.toIpPort(), std::string("0.0.0.0:1234"));
  EXPECT_EQ(addr0.port(), 1234);

  InetAddress addr1(4321, true);
  EXPECT_EQ(addr1.toIp(), std::string("127.0.0.1"));
  EXPECT_EQ(addr1.toIpPort(), std::string("127.0.0.1:4321"));
  EXPECT_EQ(addr1.port(), 4321);

  InetAddress addr2("1.2.3.4", 8888);
  EXPECT_EQ(addr2.toIp(), std::string("1.2.3.4"));
  EXPECT_EQ(addr2.toIpPort(), std::string("1.2.3.4:8888"));
  EXPECT_EQ(addr2.port(), 8888);

  InetAddress addr3("255.254.253.252", 65535);
  EXPECT_EQ(addr3.toIp(), std::string("255.254.253.252"));
  EXPECT_EQ(addr3.toIpPort(), std::string("255.254.253.252:65535"));
  EXPECT_EQ(addr3.port(), 65535);
}

TEST(InetAddress, IPv6) {
  InetAddress addr0(1234, false, true);
  EXPECT_EQ(addr0.toIp(), std::string("::"));
  EXPECT_EQ(addr0.toIpPort(), std::string("[::]:1234"));
  EXPECT_EQ(addr0.port(), 1234);

  InetAddress addr1(1234, true, true);
  EXPECT_EQ(addr1.toIp(), std::string("::1"));
  EXPECT_EQ(addr1.toIpPort(), std::string("[::1]:1234"));
  EXPECT_EQ(addr1.port(), 1234);

  InetAddress addr2("2001:db8::1", 8888, true);
  EXPECT_EQ(addr2.toIp(), std::string("2001:db8::1"));
  EXPECT_EQ(addr2.toIpPort(), std::string("[2001:db8::1]:8888"));
  EXPECT_EQ(addr2.port(), 8888);

  InetAddress addr3("fe80::1234:abcd:1", 8888);
  EXPECT_EQ(addr3.toIp(), std::string("fe80::1234:abcd:1"));
  EXPECT_EQ(addr3.toIpPort(), std::string("[fe80::1234:abcd:1]:8888"));
  EXPECT_EQ(addr3.port(), 8888);
}

TEST(InetAddress, Resolve) {
  InetAddress addr(80);
  if (InetAddress::resolve("google.com", &addr)) {
    GTEST_LOG_(INFO) << "google.com resolved to " << addr.toIpPort();
  } else {
    GTEST_LOG_(ERROR) << "Unable to resolve google.com";
    FAIL();
  }
}
