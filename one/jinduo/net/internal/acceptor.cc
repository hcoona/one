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

#include <asm-generic/errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/inet_address.h"
#include "one/jinduo/net/internal/sockets_ops.h"

namespace jinduo {
namespace net {

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_address,
                   bool reuse_port)
    : loop_(loop),
      accept_socket_(sockets::createNonblockingOrDie(listen_address.family())),
      accept_channel_(loop, accept_socket_.fd()),
      idle_fd_(::open("/dev/null",
                      O_RDONLY | O_CLOEXEC)) {  // NOLINT(hicpp-signed-bitwise)
  assert(idle_fd_ >= 0);
  accept_socket_.setReuseAddr(true);
  accept_socket_.setReusePort(reuse_port);
  accept_socket_.bindAddress(listen_address);
  accept_channel_.SetReadCallback(
      absl::bind_front(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor() {
  accept_channel_.DisableAll();
  accept_channel_.RemoveFromOwnerEventLoop();
  ::close(idle_fd_);
}

void Acceptor::Listen() {
  loop_->AssertInLoopThread();

  listening_.store(true, std::memory_order_release);
  accept_socket_.listen();

  accept_channel_.EnableReading();
}

void Acceptor::HandleRead(absl::Time /*receive_time*/) {
  loop_->AssertInLoopThread();

  InetAddress peer_address;
  while (true) {
    int connfd = accept_socket_.accept(&peer_address);
    if (connfd >= 0) {
      if (new_connection_callback_) {
        new_connection_callback_(connfd, peer_address);
      } else {
        sockets::close(connfd);
      }
    } else {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }

      LOG(ERROR) << "in Acceptor::handleRead";
      // Read the section named "The special problem of
      // accept()ing when you can't" in libev's doc.
      // By Marc Lehmann, author of libev.
      //
      // Open /dev/null to preserve a fd. When `accept()` failed because of
      // exhausted fd, we can close the preserved fd to accept the connection,
      // while just hang the connection to avoid overloaded, and then open
      // /dev/null again for fd preserving.
      if (errno == EMFILE || errno == ENFILE) {
        ::close(idle_fd_);
        idle_fd_ = ::accept(accept_socket_.fd(), nullptr, nullptr);
        ::close(idle_fd_);
        idle_fd_ =
            ::open("/dev/null",
                   O_RDONLY | O_CLOEXEC);  // NOLINT(hicpp-signed-bitwise)
      }
    }
  }
}

}  // namespace net
}  // namespace jinduo
