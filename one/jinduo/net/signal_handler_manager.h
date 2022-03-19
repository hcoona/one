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

#include <cstdint>
#include <functional>
#include <unordered_map>

#include "absl/time/time.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/internal/channel.h"

namespace jinduo {
namespace net {

// Turn signal into events. You must use it in a very begining postition in
// `main()` function to block signals in created child threads. Especially you
// need to handle it before creating `EventLoopThread` & `EventLoopThreadPool`.
//
// If your program would launch threads before `main()`, you need to register
// sigactions to forward the signal received from these threads to the main
// thread. You will miss the signals if not do so, because Linux would send
// signals to an arbitrary thread for signaling a process.
class SignalHandlerManager {
 public:
  using SignalCallback = std::function<void(int /*signum*/)>;

  explicit SignalHandlerManager(EventLoop* loop);
  ~SignalHandlerManager();

  // Disallow copy.
  SignalHandlerManager(const SignalHandlerManager&) noexcept = delete;
  SignalHandlerManager& operator=(const SignalHandlerManager&) noexcept =
      delete;

  // Disallow move.
  SignalHandlerManager(SignalHandlerManager&&) noexcept = delete;
  SignalHandlerManager& operator=(SignalHandlerManager&&) noexcept = delete;

  // Set signal callback. Newly set callback would overwrite the previously set
  // callback quietly.
  //
  // It would crash if any failure happened because we can do very little things
  // handling the signal registering error & the program must register signal
  // handlers at a very beginning point to allow all child threads inherit the
  // signal handling logics.
  void SetSignalCallback(int signum, SignalCallback callback);

 private:
  void HandleRead(absl::Time /*receive_time*/);

  EventLoop* loop_;
  int fd_;
  Channel channel_;

  std::unordered_map<int /*signum*/, SignalCallback> callbacks_;
};

}  // namespace net
}  // namespace jinduo
