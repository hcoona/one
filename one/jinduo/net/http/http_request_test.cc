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

#include "one/jinduo/net/http/http_request.h"

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "gtest/gtest.h"
#include "one/jinduo/net/buffer.h"
#include "one/jinduo/net/http/http_context.h"

using jinduo::net::Buffer;
using jinduo::net::HttpContext;
using jinduo::net::HttpRequest;

TEST(HttpRequest, ParseRequestAllInOne) {
  HttpContext context;
  Buffer input;
  input.append(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.chenshuo.com\r\n"
      "\r\n");

  EXPECT_TRUE(context.parseRequest(&input, absl::Now()));
  EXPECT_TRUE(context.gotAll());
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::kGet);
  EXPECT_EQ(request.path(), std::string("/index.html"));
  EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), std::string("www.chenshuo.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), std::string(""));
}

TEST(HttpRequest, ParseRequestInTwoPieces) {
  std::string all(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.chenshuo.com\r\n"
      "\r\n");

  for (size_t sz1 = 0; sz1 < all.size(); ++sz1) {
    HttpContext context;
    Buffer input;
    input.append(all.c_str(), sz1);
    EXPECT_TRUE(context.parseRequest(&input, absl::Now()));
    EXPECT_TRUE(!context.gotAll());

    size_t sz2 = all.size() - sz1;
    input.append(all.c_str() + sz1, sz2);
    EXPECT_TRUE(context.parseRequest(&input, absl::Now()));
    EXPECT_TRUE(context.gotAll());
    const HttpRequest& request = context.request();
    EXPECT_EQ(request.method(), HttpRequest::kGet);
    EXPECT_EQ(request.path(), std::string("/index.html"));
    EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
    EXPECT_EQ(request.getHeader("Host"), std::string("www.chenshuo.com"));
    EXPECT_EQ(request.getHeader("User-Agent"), std::string(""));
  }
}

TEST(HttpRequest, ParseRequestEmptyHeaderValue) {
  HttpContext context;
  Buffer input;
  input.append(
      "GET /index.html HTTP/1.1\r\n"
      "Host: www.chenshuo.com\r\n"
      "User-Agent:\r\n"
      "Accept-Encoding: \r\n"
      "\r\n");

  EXPECT_TRUE(context.parseRequest(&input, absl::Now()));
  EXPECT_TRUE(context.gotAll());
  const HttpRequest& request = context.request();
  EXPECT_EQ(request.method(), HttpRequest::kGet);
  EXPECT_EQ(request.path(), std::string("/index.html"));
  EXPECT_EQ(request.getVersion(), HttpRequest::kHttp11);
  EXPECT_EQ(request.getHeader("Host"), std::string("www.chenshuo.com"));
  EXPECT_EQ(request.getHeader("User-Agent"), std::string(""));
  EXPECT_EQ(request.getHeader("Accept-Encoding"), std::string(""));
}
