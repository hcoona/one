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

#include "one/jinduo/net/http/http_server.h"

#include <utility>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/net/http/http_context.h"
#include "one/jinduo/net/http/http_request.h"
#include "one/jinduo/net/http/http_response.h"

namespace jinduo {
namespace net {

namespace detail {

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}  // namespace detail

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                       std::string name, TcpServer::Option option)
    : server_(loop, listenAddr, std::move(name), option),
      httpCallback_(detail::defaultHttpCallback) {
  server_.setConnectionCallback(&HttpServer::onConnection);
  server_.setMessageCallback(absl::bind_front(&HttpServer::onMessage, this));
}

void HttpServer::start() {
  LOG(INFO) << "HttpServer[" << server_.name() << "] starts listening on "
            << server_.ipPort();
  server_.start();
}

void HttpServer::onConnection(const std::shared_ptr<TcpConnection>& conn) {
  if (conn->connected()) {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onMessage(const std::shared_ptr<TcpConnection>& conn,
                           Buffer* buf, absl::Time receiveTime) {
  HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());

  if (!context->parseRequest(buf, receiveTime)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll()) {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const std::shared_ptr<TcpConnection>& conn,
                           const HttpRequest& req) {
  const std::string& connection = req.getHeader("Connection");
  bool close =
      connection == "close" ||
      (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection()) {
    conn->shutdown();
  }
}

}  // namespace net
}  // namespace jinduo
