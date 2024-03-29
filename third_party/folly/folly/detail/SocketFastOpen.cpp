/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "folly/detail/SocketFastOpen.h"

#include <cerrno>
#include <cstdio>
#include <fstream>

#include "folly/portability/Sockets.h"

namespace folly {
namespace detail {

#if FOLLY_ALLOW_TFO && defined(__linux__)

// Sometimes these flags are not present in the headers,
// so define them if not present.
#if !defined(MSG_FASTOPEN)
#define MSG_FASTOPEN 0x20000000
#endif

#if !defined(TCP_FASTOPEN)
#define TCP_FASTOPEN 23
#endif

#if !defined(TCPI_OPT_SYN_DATA)
#define TCPI_OPT_SYN_DATA 32
#endif

ssize_t tfo_sendmsg(NetworkSocket sockfd, const struct msghdr* msg, int flags) {
  flags |= MSG_FASTOPEN;
  return netops::sendmsg(sockfd, msg, flags);
}

int tfo_enable(NetworkSocket sockfd, size_t max_queue_size) {
  return netops::setsockopt(
      sockfd, SOL_TCP, TCP_FASTOPEN, &max_queue_size, sizeof(max_queue_size));
}

bool tfo_succeeded(NetworkSocket sockfd) {
  // Call getsockopt to check if TFO was used.
  struct tcp_info info;
  socklen_t info_len = sizeof(info);
  errno = 0;
  if (netops::getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, &info, &info_len) !=
      0) {
    // errno is set from getsockopt
    return false;
  }
  return info.tcpi_options & TCPI_OPT_SYN_DATA;
}

PlatformTFOSettings tfo_platform_availability() {
  static PlatformTFOSettings TFOSettings = [] {
    size_t fastOpen = 0;
    try {
      std::ifstream ifs("/proc/sys/net/ipv4/tcp_fastopen");
      if (ifs.is_open()) {
        ifs >> fastOpen;
      }
    } catch (std::exception&) {
    }

    PlatformTFOSettings settings{};
    if ((fastOpen & 1) == 0) {
      settings.client = TFOAvailability::None;
    } else if ((fastOpen & 4) == 0) {
      settings.client = TFOAvailability::WithCookies;
    } else {
      settings.client = TFOAvailability::Unconditional;
    }

    if ((fastOpen & 2) == 0) {
      settings.server = TFOAvailability::None;
    } else if ((fastOpen & 0x200) == 0) {
      settings.server = TFOAvailability::WithCookies;
    } else {
      settings.server = TFOAvailability::Unconditional;
    }

    return settings;
  }();

  return TFOSettings;
}

#elif FOLLY_ALLOW_TFO && defined(__APPLE__)

ssize_t tfo_sendmsg(NetworkSocket sockfd, const struct msghdr* msg, int flags) {
  sa_endpoints_t endpoints;
  endpoints.sae_srcif = 0;
  endpoints.sae_srcaddr = nullptr;
  endpoints.sae_srcaddrlen = 0;
  endpoints.sae_dstaddr = (struct sockaddr*)msg->msg_name;
  endpoints.sae_dstaddrlen = msg->msg_namelen;
  int ret = connectx(
      sockfd.toFd(),
      &endpoints,
      SAE_ASSOCID_ANY,
      CONNECT_RESUME_ON_READ_WRITE | CONNECT_DATA_IDEMPOTENT,
      nullptr,
      0,
      nullptr,
      nullptr);

  if (ret != 0) {
    return ret;
  }
  ret = netops::sendmsg(sockfd, msg, flags);
  return ret;
}

int tfo_enable(NetworkSocket sockfd, size_t max_queue_size) {
  return netops::setsockopt(
      sockfd,
      IPPROTO_TCP,
      TCP_FASTOPEN,
      &max_queue_size,
      sizeof(max_queue_size));
}

bool tfo_succeeded(NetworkSocket /* sockfd */) {
  errno = EOPNOTSUPP;
  return false;
}

PlatformTFOSettings tfo_platform_availability() {
  static PlatformTFOSettings TFOSettings{
      TFOAvailability::None, TFOAvailability::None};
  return TFOSettings;
}

#else

ssize_t tfo_sendmsg(
    NetworkSocket /* sockfd */,
    const struct msghdr* /* msg */,
    int /* flags */) {
  errno = EOPNOTSUPP;
  return -1;
}

int tfo_enable(NetworkSocket /* sockfd */, size_t /* max_queue_size */) {
  errno = ENOPROTOOPT;
  return -1;
}

bool tfo_succeeded(NetworkSocket /* sockfd */) {
  errno = EOPNOTSUPP;
  return false;
}

PlatformTFOSettings tfo_platform_availability() {
  static PlatformTFOSettings TFOSettings{
      TFOAvailability::None, TFOAvailability::None};
  return TFOSettings;
}

#endif
} // namespace detail
} // namespace folly
