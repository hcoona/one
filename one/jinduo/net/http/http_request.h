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

#include <assert.h>
#include <stdio.h>

#include <map>
#include <string>
#include <utility>

#include "absl/time/time.h"

namespace jinduo {
namespace net {

class HttpRequest {
 public:
  enum Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
  enum Version { kUnknown, kHttp10, kHttp11 };

  void setVersion(Version v) { version_ = v; }

  [[nodiscard]] Version getVersion() const { return version_; }

  bool setMethod(const char* start, const char* end) {
    assert(method_ == kInvalid);
    std::string m(start, end);
    if (m == "GET") {
      method_ = kGet;
    } else if (m == "POST") {
      method_ = kPost;
    } else if (m == "HEAD") {
      method_ = kHead;
    } else if (m == "PUT") {
      method_ = kPut;
    } else if (m == "DELETE") {
      method_ = kDelete;
    } else {
      method_ = kInvalid;
    }
    return method_ != kInvalid;
  }

  [[nodiscard]] Method method() const { return method_; }

  [[nodiscard]] const char* methodString() const {
    const char* result = "UNKNOWN";
    switch (method_) {
      case kGet:
        result = "GET";
        break;
      case kPost:
        result = "POST";
        break;
      case kHead:
        result = "HEAD";
        break;
      case kPut:
        result = "PUT";
        break;
      case kDelete:
        result = "DELETE";
        break;
      default:
        break;
    }
    return result;
  }

  void setPath(const char* start, const char* end) { path_.assign(start, end); }

  [[nodiscard]] const std::string& path() const { return path_; }

  void setQuery(const char* start, const char* end) {
    query_.assign(start, end);
  }

  [[nodiscard]] const std::string& query() const { return query_; }

  void setReceiveTime(absl::Time t) { receiveTime_ = t; }

  [[nodiscard]] absl::Time receiveTime() const { return receiveTime_; }

  void addHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && (::isspace(*colon) != 0)) {
      ++colon;
    }
    std::string value(colon, end);
    while (!value.empty() && (::isspace(value[value.size() - 1]) != 0)) {
      value.resize(value.size() - 1);
    }
    headers_[field] = value;
  }

  [[nodiscard]] std::string getHeader(const std::string& field) const {
    std::string result;
    auto it = headers_.find(field);
    if (it != headers_.end()) {
      result = it->second;
    }
    return result;
  }

  [[nodiscard]] const std::map<std::string, std::string>& headers() const {
    return headers_;
  }

  void swap(HttpRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    std::swap(receiveTime_, that.receiveTime_);
    headers_.swap(that.headers_);
  }

 private:
  Method method_{kInvalid};
  Version version_{kUnknown};
  std::string path_{};
  std::string query_{};
  absl::Time receiveTime_{};
  std::map<std::string, std::string> headers_{};
};

}  // namespace net
}  // namespace jinduo
