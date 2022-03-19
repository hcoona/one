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

#include <csignal>
#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <thread>

#include "absl/base/casts.h"
#include "absl/functional/bind_front.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/codelab/minikafka/core/kafka_service.h"
#include "one/codelab/minikafka/transport/kafka_tcp_server.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/event_loop_thread.h"
#include "one/jinduo/net/event_loop_thread_pool.h"
#include "one/jinduo/net/signal_handler_manager.h"

namespace {

bool ValidateTcpPort(const char* /*flag_name*/, uint32_t value) {
  return value <=
         absl::implicit_cast<uint32_t>(std::numeric_limits<uint16_t>::max());
}

void HandleSignal(
    jinduo::net::EventLoop* loop,
    hcoona::minikafka::KafkaService* kafka_service,
    std::vector<hcoona::minikafka::KafkaTcpServer*>* kafka_tcp_servers,
    int signum) {
  CHECK(signum == SIGTERM || signum == SIGINT);

  LOG(WARNING) << "SIGTERM|SIGINT received, exiting...";

  // The events are drained during event loop quiting.
  loop->QueueInLoop(absl::bind_front(&jinduo::net::EventLoop::Quit, loop));

  // Child loops exited during kafka tcp server stopping.
  for (auto&& s : *kafka_tcp_servers) {
    s->Stop();
  }

  // TODO(zhangshuai.ustc): graceful shutdown the service.
  (void)kafka_service;
}

}  // namespace

DEFINE_uint32(port, 0, "Kafka plain protocol binding port.");
DEFINE_uint32(inspector_port, 0, "Inspector HTTP server binding port.");

DEFINE_validator(port, &ValidateTcpPort);
DEFINE_validator(inspector_port, &ValidateTcpPort);

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  // TODO(zhangshuai.ustc): Bind a HTTP server to inspect the state of the
  // service.

  jinduo::net::EventLoop loop;
  jinduo::net::EventLoopThreadPool event_loop_thread_pool(&loop, "minikafka");
  event_loop_thread_pool.set_thread_num(
      static_cast<int>(std::thread::hardware_concurrency()));

  hcoona::minikafka::KafkaService kafka_service;
  std::vector<hcoona::minikafka::KafkaTcpServer*> kafka_tcp_servers;

  jinduo::net::SignalHandlerManager signal_handler_manager(&loop);
  signal_handler_manager.SetSignalCallback(
      SIGINT, absl::bind_front(&HandleSignal, &loop, &kafka_service,
                               &kafka_tcp_servers));
  signal_handler_manager.SetSignalCallback(
      SIGTERM, absl::bind_front(&HandleSignal, &loop, &kafka_service,
                                &kafka_tcp_servers));

  event_loop_thread_pool.Start();

  kafka_tcp_servers.reserve(std::thread::hardware_concurrency());
  for (jinduo::net::EventLoop* child_loop :
       event_loop_thread_pool.all_loops()) {
    auto kafka_tcp_server = std::make_shared<hcoona::minikafka::KafkaTcpServer>(
        &kafka_service, child_loop, jinduo::net::InetAddress(FLAGS_port));
    kafka_tcp_servers.emplace_back(kafka_tcp_server.get());
    child_loop->RunInLoop([s = kafka_tcp_server.get()] { s->Start(); });
    // It's safe to store `std::shared_ptr` in `std::any`.
    child_loop->set_context(std::move(kafka_tcp_server));
  }

  LOG(INFO) << "Minikafka service is running at port " << FLAGS_port;

  loop.Loop();

  // TODO(zhangshuai.ustc): Join `kafka_service`.
}
