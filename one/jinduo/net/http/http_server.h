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

#include <memory>
#include <string>

#include "one/jinduo/net/tcp_server.h"

namespace jinduo {
namespace net {

class HttpRequest;
class HttpResponse;

/// A simple embedded HTTP server designed for report status of a program.
/// It is not a fully HTTP 1.1 compliant server, but provides minimum features
/// that can communicate with HttpClient and Web browser.
/// It is synchronous, just like Java Servlet.
class HttpServer {
 public:
  using HttpCallback = std::function<void(const HttpRequest&, HttpResponse*)>;

  HttpServer(EventLoop* loop, const InetAddress& listenAddr, std::string name,
             TcpServer::Option option = TcpServer::kNoReusePort);
  ~HttpServer() = default;

  // Disallow copy.
  HttpServer(const HttpServer&) noexcept = delete;
  HttpServer& operator=(const HttpServer&) noexcept = delete;

  // Allow move but not implemented yet.
  HttpServer(HttpServer&&) noexcept = delete;
  HttpServer& operator=(HttpServer&&) noexcept = delete;

  EventLoop* getLoop() const { return server_.getLoop(); }

  /// Not thread safe, callback be registered before calling start().
  void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void start();

 private:
  static void onConnection(const std::shared_ptr<TcpConnection>& conn);
  void onMessage(const std::shared_ptr<TcpConnection>& conn, Buffer* buf,
                 absl::Time receiveTime);
  void onRequest(const std::shared_ptr<TcpConnection>&, const HttpRequest&);

  TcpServer server_;
  HttpCallback httpCallback_;
};

}  // namespace net
}  // namespace jinduo
