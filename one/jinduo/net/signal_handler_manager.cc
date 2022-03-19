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

#include "one/jinduo/net/signal_handler_manager.h"

#include <sys/signalfd.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstring>
#include <utility>

#include "absl/functional/bind_front.h"
#include "glog/logging.h"
#include "one/jinduo/base/strerror.h"

namespace jinduo {
namespace net {

namespace {

sigset_t CreateEmptySignalSetOrDie() {
  sigset_t set;
  ::memset(&set, 0, sizeof(set));
  PLOG_IF(FATAL, ::sigemptyset(&set) != 0)
      << "Failed to create empty signal set.";
  return set;
}

int CreateEmptySignalFdOrDie() {
  static constexpr int kCreateNewFd = -1;
  static constexpr int kInvalidFd = -1;

  sigset_t set = CreateEmptySignalSetOrDie();
  int fd = ::signalfd(kCreateNewFd, &set, SFD_CLOEXEC | SFD_NONBLOCK);
  PLOG_IF(FATAL, fd == kInvalidFd) << "Failed to create empty signal fd.";

  return fd;
}

}  // namespace

SignalHandlerManager::SignalHandlerManager(EventLoop* loop)
    : loop_(loop), fd_(CreateEmptySignalFdOrDie()), channel_(loop_, fd_) {
  channel_.SetReadCallback(
      absl::bind_front(&SignalHandlerManager::HandleRead, this));
  loop_->RunInLoop([this]() { channel_.EnableReading(); });
}

SignalHandlerManager::~SignalHandlerManager() {
  channel_.DisableAll();
  channel_.RemoveFromOwnerEventLoop();
  ::close(fd_);
}

void SignalHandlerManager::SetSignalCallback(int signum,
                                             SignalCallback callback) {
  sigset_t set;
  int rc = ::pthread_sigmask(SIG_SETMASK, nullptr, &set);
  LOG_IF(FATAL, rc != 0) << "Failed to query sigset. signum=" << signum
                         << ", error=" << strerror_tl(rc);

  PLOG_IF(FATAL, sigaddset(&set, signum) != 0);
  rc = ::pthread_sigmask(SIG_SETMASK, &set, nullptr);
  LOG_IF(FATAL, rc != 0) << "Failed to block signal. signum=" << signum
                         << ", error=" << strerror_tl(rc);

  callbacks_[signum] = std::move(callback);

  PLOG_IF(FATAL, signalfd(fd_, &set, SFD_CLOEXEC | SFD_NONBLOCK) != fd_)
      << "Failed to register signal to signal fd. signum=" << signum;
}

void SignalHandlerManager::HandleRead(absl::Time /*receive_time*/) {
  loop_->AssertInLoopThread();

  static constexpr size_t kSiginfoBufferSize = 16;
  signalfd_siginfo siginfo_buffer[kSiginfoBufferSize];

  while (true) {
    ssize_t read_bytes = ::read(fd_, siginfo_buffer,
                                sizeof(signalfd_siginfo) * kSiginfoBufferSize);
    VLOG(1) << "Read " << read_bytes << " from signalfd. fd=" << fd_;
    if (read_bytes == -1) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        break;
      }

      PLOG(FATAL) << "Failed to handle signal fd reading.";
    }

    CHECK_EQ(static_cast<int>(read_bytes % sizeof(signalfd_siginfo)), 0);

    for (int i = 0; i < static_cast<int>(read_bytes / sizeof(signalfd_siginfo));
         i++) {
      auto it = callbacks_.find(static_cast<int>(siginfo_buffer[i].ssi_signo));
      if (it != callbacks_.end()) {
        it->second(it->first);
      }
    }
  }
}

}  // namespace net
}  // namespace jinduo
