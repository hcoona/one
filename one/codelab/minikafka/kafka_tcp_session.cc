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

#include "one/codelab/minikafka/kafka_tcp_session.h"

#include <limits>
#include <vector>

#include "absl/functional/bind_front.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "glog/logging.h"

namespace hcoona {
namespace minikafka {

namespace {

constexpr size_t kLeadingRequestSizeBytes = 4;

enum class KafkaTcpSessionContextState {
  kWaitingRequestLeadingSize,
  kWaitingRequestContentBytes,
};

class KafkaTcpSessionContext {
 public:
  absl::Status Parse(jinduo::net::Buffer* buffer, absl::Time receiveTime);

  [[nodiscard]] const std::vector<int>& kafka_requests() const {
    return kafka_requests_;
  }

  std::vector<int>* mutable_kafka_requests() { return &kafka_requests_; }

  void ClearKafkaRequests() { kafka_requests_.clear(); }

  void reset();

 private:
  KafkaTcpSessionContextState state_{
      KafkaTcpSessionContextState::kWaitingRequestLeadingSize};
  int32_t known_request_leading_size_{
      std::numeric_limits<decltype(known_request_leading_size_)>::min()};
  std::vector<int> kafka_requests_{};
};

// Return UNAVAILABLE for waiting more data from buffer.
//
// https://kafka.apache.org/protocol#protocol_common
//
// RequestOrResponse => Size (RequestMessage | ResponseMessage)
// Size => int32
//
// The message_size field gives the size of the subsequent request or response
// message in bytes. The client can read requests by first reading this 4 byte
// size as an integer N, and then reading and parsing the subsequent N bytes of
// the request.
absl::Status KafkaTcpSessionContext::Parse(jinduo::net::Buffer* buffer,
                                           absl::Time receiveTime) {
  while (true) {
    if (state_ == KafkaTcpSessionContextState::kWaitingRequestLeadingSize) {
      if (buffer->readableBytes() < kLeadingRequestSizeBytes) {
        return absl::UnavailableError("No enough bytes for the request.");
      }

      known_request_leading_size_ = buffer->readInt32();
      if (known_request_leading_size_ < 0) {
        return absl::UnknownError(absl::StrCat(
            "Unexpected request leading size: ", known_request_leading_size_));
      }

      state_ = KafkaTcpSessionContextState::kWaitingRequestContentBytes;
    }

    if (state_ == KafkaTcpSessionContextState::kWaitingRequestContentBytes) {
      CHECK_GE(known_request_leading_size_, 0);
      if (buffer->readableBytes() <
          static_cast<size_t>(known_request_leading_size_)) {
        return absl::UnavailableError("No enough bytes for the request.");
      }

      // TODO(zhangshuai.ds): Parse the request.
      (void)buffer->toStringView().substr(0, known_request_leading_size_);
      (void)receiveTime;

      known_request_leading_size_ =
          std::numeric_limits<decltype(known_request_leading_size_)>::min();

      state_ = KafkaTcpSessionContextState::kWaitingRequestLeadingSize;
    }
  }
}

}  // namespace

KafkaTcpSession::KafkaTcpSession(
    std::shared_ptr<jinduo::net::TcpConnection> connection)
    : connection_(std::move(connection)) {
  connection_->setTcpNoDelay(/*on=*/true);
  connection_->setContext(KafkaTcpSessionContext());
  connection_->setMessageCallback(
      absl::bind_front(&KafkaTcpSession::OnMessage, this));
}

void KafkaTcpSession::OnMessage(
    const std::shared_ptr<jinduo::net::TcpConnection>& connection,
    jinduo::net::Buffer* buffer, absl::Time receiveTime) {
  {
    absl::MutexLock lock(&mutex_);
    last_active_time_ = receiveTime;
  }

  KafkaTcpSessionContext* context =
      std::any_cast<KafkaTcpSessionContext>(connection->getMutableContext());

  absl::Status s = context->Parse(buffer, receiveTime);
  if (!s.ok() && !absl::IsUnavailable(s)) {
    LOG(WARNING) << "Failed to parse request. remote="
                 << connection->peerAddress().toIpPort() << ", reason=" << s;
    connection->shutdown();
  }

  if (!context->kafka_requests().empty()) {
    for (auto&& request : *context->mutable_kafka_requests()) {
      (void)request;
      // OnRequest(conn, std::move(request));
    }

    context->ClearKafkaRequests();
  }
}

}  // namespace minikafka
}  // namespace hcoona
