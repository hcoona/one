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

#pragma once

#include <memory>
#include <utility>
#include <vector>

#include "absl/synchronization/mutex.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "one/codelab/minikafka/core/kafka_service.h"
#include "one/jinduo/net/buffer.h"
#include "one/jinduo/net/tcp_connection.h"

namespace hcoona {
namespace minikafka {

class KafkaTcpSession {
 public:
  KafkaTcpSession(KafkaService* kafka_service_,
                  std::shared_ptr<jinduo::net::TcpConnection> connection);

  [[nodiscard]] absl::Time last_active_time() const {
    absl::MutexLock lock(&mutex_);
    return last_active_time_;
  }

 private:
  void OnMessage(
      const std::shared_ptr<jinduo::net::TcpConnection>& /*connection*/,
      jinduo::net::Buffer* buffer, absl::Time receive_time);
  void ProcessRequests(std::vector<RequestHeaderAndBody>&& requests);

  KafkaService* kafka_service_;
  std::shared_ptr<jinduo::net::TcpConnection> connection_;

  mutable absl::Mutex mutex_;
  absl::Time last_active_time_ ABSL_GUARDED_BY(mutex_){absl::Now()};
};

}  // namespace minikafka
}  // namespace hcoona
