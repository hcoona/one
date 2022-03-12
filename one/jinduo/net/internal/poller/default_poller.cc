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

#include <stdlib.h>

#include "one/jinduo/net/internal/poller.h"
#include "one/jinduo/net/internal/poller/epoll_poller.h"
#include "one/jinduo/net/internal/poller/poll_poller.h"

namespace jinduo {
namespace net {

Poller* Poller::newDefaultPoller(EventLoop* loop) {
  if (::getenv("JINDUO_USE_POLL") != nullptr) {
    return new PollPoller(loop);
  }
  return new EPollPoller(loop);
}

}  // namespace net
}  // namespace jinduo
