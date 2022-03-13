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

#include <map>
#include <string>

namespace jinduo {
namespace net {

class Buffer;

class HttpResponse {
 public:
  enum HttpStatusCode {
    kUnknown,
    k200Ok = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
  };

  explicit HttpResponse(bool close) : closeConnection_(close) {}

  void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

  void setStatusMessage(const std::string& message) {
    statusMessage_ = message;
  }

  void setCloseConnection(bool on) { closeConnection_ = on; }

  [[nodiscard]] bool closeConnection() const { return closeConnection_; }

  void setContentType(const std::string& contentType) {
    addHeader("Content-Type", contentType);
  }

  // FIXME: replace std::string with StringPiece
  void addHeader(const std::string& key, const std::string& value) {
    headers_[key] = value;
  }

  void setBody(const std::string& body) { body_ = body; }

  void appendToBuffer(Buffer* output) const;

 private:
  std::map<std::string, std::string> headers_;
  HttpStatusCode statusCode_{kUnknown};
  // FIXME: add http version
  std::string statusMessage_;
  bool closeConnection_;
  std::string body_;
};

}  // namespace net
}  // namespace jinduo
