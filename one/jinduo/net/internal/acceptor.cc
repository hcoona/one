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
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.
//
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/net/internal/acceptor.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/inet_address.h"
#include "one/jinduo/net/internal/sockets_ops.h"

namespace jinduo {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr,
                   bool reuseport)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
      acceptChannel_(loop, acceptSocket_.fd()),
      listening_(false),
      idleFd_(::open("/dev/null",
                     O_RDONLY | O_CLOEXEC)) {  // NOLINT(hicpp-signed-bitwise)
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.SetReadCallback([this](absl::Time) { handleRead(); });
}

Acceptor::~Acceptor() {
  acceptChannel_.DisableAll();
  acceptChannel_.RemoveFromOwnerEventLoop();
  ::close(idleFd_);
}

void Acceptor::listen() {
  loop_->AssertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.EnableReading();
}

void Acceptor::handleRead() {
  loop_->AssertInLoopThread();
  InetAddress peerAddr;
  // FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
    // string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    if (newConnectionCallback_) {
      newConnectionCallback_(connfd, peerAddr);
    } else {
      sockets::close(connfd);
    }
  } else {
    LOG(ERROR) << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE) {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), nullptr, nullptr);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null",
                       O_RDONLY | O_CLOEXEC);  // NOLINT(hicpp-signed-bitwise)
    }
  }
}

}  // namespace net
}  // namespace jinduo
