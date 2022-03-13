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

#include "one/jinduo/net/http/http_context.h"

#include "one/jinduo/net/buffer.h"

namespace jinduo {
namespace net {

bool HttpContext::processRequestLine(const char* begin, const char* end) {
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  if (space != end && request_.setMethod(start, space)) {
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space != end) {
      const char* question = std::find(start, space, '?');
      if (question != space) {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      } else {
        request_.setPath(start, space);
      }
      start = space + 1;
      succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
      if (succeed) {
        if (*(end - 1) == '1') {
          request_.setVersion(HttpRequest::kHttp11);
        } else if (*(end - 1) == '0') {
          request_.setVersion(HttpRequest::kHttp10);
        } else {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

// return false if any error
bool HttpContext::parseRequest(Buffer* buf, absl::Time receiveTime) {
  bool ok = true;
  bool hasMore = true;
  while (hasMore) {
    if (state_ == kExpectRequestLine) {
      const char* crlf = buf->findCRLF();
      if (crlf != nullptr) {
        ok = processRequestLine(buf->peek(), crlf);
        if (ok) {
          request_.setReceiveTime(receiveTime);
          buf->retrieveUntil(crlf + 2);
          state_ = kExpectHeaders;
        } else {
          hasMore = false;
        }
      } else {
        hasMore = false;
      }
    } else if (state_ == kExpectHeaders) {
      const char* crlf = buf->findCRLF();
      if (crlf != nullptr) {
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf) {
          request_.addHeader(buf->peek(), colon, crlf);
        } else {
          // empty line, end of header
          // FIXME:
          state_ = kGotAll;
          hasMore = false;
        }
        buf->retrieveUntil(crlf + 2);
      } else {
        hasMore = false;
      }
    } else if (state_ == kExpectBody) {
      // FIXME:
    }
  }
  return ok;
}

}  // namespace net
}  // namespace jinduo
