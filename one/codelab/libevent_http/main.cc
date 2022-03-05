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

#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#include <array>
#include <memory>
#include <string_view>

#include "absl/strings/cord.h"
#include "event2/buffer.h"
#include "event2/event.h"
#include "event2/http.h"
#include "event2/keyvalq_struct.h"
#include "event2/thread.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

namespace {

void RegisterEventLogCallback() {
  ::event_set_log_callback([](int severity, const char* msg) {
    switch (severity) {
      case _EVENT_LOG_DEBUG:
        VLOG(1) << msg;
        break;
      case _EVENT_LOG_MSG:
        LOG(INFO) << msg;
        break;
      case _EVENT_LOG_WARN:
        LOG(WARNING) << msg;
        break;
      case _EVENT_LOG_ERR:
        LOG(ERROR) << msg;
        break;
      default:
        LOG(FATAL) << "Should not reach here! msg=" << msg;
        break;
    }
  });
}

void DumpRequestCallback(struct evhttp_request* req, void* /*arg*/) {
  std::string_view cmdtype;
  switch (::evhttp_request_get_command(req)) {
    case EVHTTP_REQ_GET:
      cmdtype = "GET";
      break;
    case EVHTTP_REQ_POST:
      cmdtype = "POST";
      break;
    case EVHTTP_REQ_HEAD:
      cmdtype = "HEAD";
      break;
    case EVHTTP_REQ_PUT:
      cmdtype = "PUT";
      break;
    case EVHTTP_REQ_DELETE:
      cmdtype = "DELETE";
      break;
    case EVHTTP_REQ_OPTIONS:
      cmdtype = "OPTIONS";
      break;
    case EVHTTP_REQ_TRACE:
      cmdtype = "TRACE";
      break;
    case EVHTTP_REQ_CONNECT:
      cmdtype = "CONNECT";
      break;
    case EVHTTP_REQ_PATCH:
      cmdtype = "PATCH";
      break;
    default:
      cmdtype = "UNKNOWN";
      break;
  }

  LOG(INFO) << cmdtype << ":" << ::evhttp_request_get_uri(req);

  LOG(INFO) << "Headers:";
  ::evkeyvalq* headers = ::evhttp_request_get_input_headers(req);
  for (::evkeyval* header = headers->tqh_first; header != nullptr;
       header = header->next.tqe_next) {
    LOG(INFO) << "  " << header->key << "=" << header->value;
  }

  LOG(INFO) << "Input data:";
  absl::Cord data_buffer;
  ::evbuffer* buf = ::evhttp_request_get_input_buffer(req);
  while (::evbuffer_get_length(buf)) {
    static constexpr size_t kBufferSize = 128;
    char cbuf[kBufferSize];
    int n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
    data_buffer.Append(std::string_view(cbuf, n));
  }
  LOG(INFO) << "\n" << data_buffer;

  ::evhttp_send_reply(req, HTTP_OK, "OK", /*databuf=*/nullptr);
}

void HandleHttpRequest(::evhttp_request* request, void* /*arg*/) {
  DumpRequestCallback(request, /*arg=*/nullptr);
}

void PrintBindingSocket(::evhttp_bound_socket* socket) {
  evutil_socket_t fd = evhttp_bound_socket_get_fd(socket);

  ::sockaddr_storage ss{};
  ev_socklen_t socklen = sizeof(ss);
  memset(&ss, 0, sizeof(ss));
  CHECK_EQ(0, getsockname(fd, reinterpret_cast<::sockaddr*>(&ss), &socklen));

  int got_port{};
  void* inaddr;
  if (ss.ss_family == AF_INET) {
    got_port = ntohs((reinterpret_cast<::sockaddr_in*>(&ss))->sin_port);
    inaddr = &(reinterpret_cast<::sockaddr_in*>(&ss))->sin_addr;
  } else if (ss.ss_family == AF_INET6) {
    got_port = ntohs((reinterpret_cast<::sockaddr_in6*>(&ss))->sin6_port);
    inaddr = &(reinterpret_cast<::sockaddr_in6*>(&ss))->sin6_addr;
  } else {
    LOG(FATAL) << "Weird address family: " << ss.ss_family;
  }

  char addrbuf[INET6_ADDRSTRLEN];
  const char* addr = CHECK_NOTNULL(
      ::evutil_inet_ntop(ss.ss_family, inaddr, addrbuf, sizeof(addrbuf)));
  LOG(INFO) << "Listening on " << addr << ":" << got_port;
}

}  // namespace

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  ::evthread_use_pthreads();

  RegisterEventLogCallback();

  auto cfg = std::unique_ptr<::event_config, decltype(&::event_config_free)>(
      ::event_config_new(), &::event_config_free);
  CHECK(static_cast<bool>(cfg)) << "Failed to new libevent config.";

  ::event_config_avoid_method(cfg.get(), "select");
  ::event_config_require_features(cfg.get(), EV_FEATURE_ET);
  ::event_config_set_flag(cfg.get(), EVENT_BASE_FLAG_EPOLL_USE_CHANGELIST);

  auto base = std::unique_ptr<::event_base, decltype(&::event_base_free)>(
      ::event_base_new_with_config(cfg.get()), &::event_base_free);

  auto http = std::unique_ptr<::evhttp, decltype(&::evhttp_free)>(
      ::evhttp_new(base.get()), &::evhttp_free);

  ::evhttp_set_gencb(http.get(), &HandleHttpRequest, /*arg=*/nullptr);

  ::evhttp_bound_socket* socket =
      CHECK_NOTNULL(::evhttp_bind_socket_with_handle(http.get(), "::", 0));
  PrintBindingSocket(socket);

  ::event* term = CHECK_NOTNULL(evsignal_new(  // NOLINT(hicpp-signed-bitwise)
      base.get(), SIGINT,
      [](int /*sig*/, short /*events*/,  // NOLINT(runtime/int): C API.
         void* arg) {
        auto base = reinterpret_cast<::event_base*>(arg);
        ::event_base_loopbreak(base);
        LOG(WARNING) << "Exiting...";
      },
      base.get()));
  CHECK_EQ(0, event_add(term, /*timeout=*/nullptr));

  ::event_base_dispatch(base.get());

  return 0;
}
