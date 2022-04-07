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

#include <utility>

#include "folly/io/async/AsyncServerSocket.h"
#include "folly/io/async/AsyncSocket.h"
#include "folly/io/async/EventBase.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

class Acceptor : public folly::AsyncServerSocket::AcceptCallback {
 public:
  void connectionAccepted(folly::NetworkSocket fd,
                          const folly::SocketAddress& clientAddr,
                          AcceptInfo /*info*/) noexcept override {
    auto socket =
        folly::AsyncSocket::newSocket(worker_event_base_, fd, &clientAddr);
    // TODO(zhangshuai.ustc): set socket options.
    socket->setCloseOnExec();
    // TODO(zhangshuai.ustc): move to connection for management.
  }

 private:
  folly::EventBase* worker_event_base_;
};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  folly::EventBase::Options event_base_options;
  folly::EventBase acceptor_event_base(event_base_options);

  auto server_socket =
      folly::AsyncServerSocket::newSocket(&acceptor_event_base);
  server_socket->setReusePortEnabled(/*enabled=*/true);
  server_socket->setCloseOnExec(/*closeOnExec=*/true);
  static constexpr uint32_t kTfoQueueSizeLimit = 100;
  server_socket->setTFOEnabled(/*enabled=*/true, kTfoQueueSizeLimit);
  static constexpr uint16_t kBindingPort = 8888;
  server_socket->bind(kBindingPort);
}
