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

#include "absl/functional/bind_front.h"
#include "absl/status/status.h"
#include "glog/logging.h"

namespace hcoona {
namespace minikafka {

namespace {

enum class KafkaTcpSessionContextState {
  kParseRequestHeader,
  kParseRequestBody,
  kParseOneRequestCompleted,
};

class KafkaTcpSessionContext {
 public:
  absl::Status Parse(jinduo::net::Buffer* buffer, absl::Time receiveTime);

  [[nodiscard]] bool parse_one_request_completed() const {
    return state_ == KafkaTcpSessionContextState::kParseOneRequestCompleted;
  }

  void reset();

 private:
  KafkaTcpSessionContextState state_{
      KafkaTcpSessionContextState::kParseRequestHeader};
};

absl::Status KafkaTcpSessionContext::Parse(jinduo::net::Buffer* buffer,
                                           absl::Time receiveTime) {
  (void)buffer;
  (void)receiveTime;
  (void)state_;
  return absl::UnimplementedError("Not implemented yet.");
}

void KafkaTcpSessionContext::reset() {
  (void)state_;
  LOG(FATAL) << "Not implemented yet.";
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
  KafkaTcpSessionContext* context =
      std::any_cast<KafkaTcpSessionContext>(connection->getMutableContext());

  absl::Status s = context->Parse(buffer, receiveTime);
  if (!s.ok()) {
    LOG(WARNING) << "Failed to parse request. remote="
                 << connection->peerAddress().toIpPort() << ", reason=" << s;
    connection->shutdown();
  }

  if (context->parse_one_request_completed()) {
    // OnRequest(conn, context->request());
    context->reset();
  }
}

}  // namespace minikafka
}  // namespace hcoona
