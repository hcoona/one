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

#include <cstddef>
#include <limits>

#include "absl/base/casts.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/codelab/minikafka/kafka_tcp_server.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/event_loop_thread.h"

DEFINE_uint32(port, 0, "Kafka plain protocol binding port.");
DEFINE_uint32(inspector_port, 0, "Inspector HTTP server binding port.");

namespace {

bool ValidateTcpPort(const char* /*flag_name*/, uint32_t value) {
  return value <=
         absl::implicit_cast<uint32_t>(std::numeric_limits<uint16_t>::max());
}

}  // namespace

DEFINE_validator(port, &ValidateTcpPort);
DEFINE_validator(inspector_port, &ValidateTcpPort);

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  // TODO(zhangshuai.ds): Turn signal into an event to help stop the event loop
  // gracefully.

  // TODO(zhangshuai.ds): Bind a HTTP server to inspect the state of the
  // service.

  // TODO(zhangshuai.ds): Launch multiple TCP servers binding on the same port
  // to process accept in parallel.

  jinduo::net::EventLoop loop;
  hcoona::minikafka::KafkaTcpServer kafka_tcp_server(
      &loop, jinduo::net::InetAddress(FLAGS_port));
  kafka_tcp_server.Start();
  loop.loop();
}
