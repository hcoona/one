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

#include <limits>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "muduo/base/Types.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/inspect/Inspector.h"

DEFINE_uint32(port, 0, "Kafka plain protocol binding port.");
DEFINE_uint32(inspector_port, 0, "Inspector HTTP server binding port.");

namespace {

bool ValidateTcpPort(const char* /*flag_name*/, uint32_t value) {
  return value <=
         muduo::implicit_cast<uint32_t>(std::numeric_limits<uint16_t>::max());
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

  muduo::net::EventLoop loop;
  muduo::net::EventLoopThread inspectThread;
  muduo::net::Inspector inspector(inspectThread.startLoop(),
                                  muduo::net::InetAddress(FLAGS_inspector_port),
                                  "minikafka-inspector");

  // MemcacheServer server(&loop, options);
  // server.setThreadNum(options.threads);
  // server.start();
  loop.loop();
}
