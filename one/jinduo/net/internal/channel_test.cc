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
//
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/net/internal/channel.h"

#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <functional>
#include <map>
#include <utility>

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"

void print(const char* msg) {
  static std::map<const char*, absl::Time> lasts;
  absl::Time& last = lasts[msg];
  absl::Time now = absl::Now();
  printf("%s tid %d %s delay %s\n", absl::FormatTime(now).c_str(),
         jinduo::this_thread::tid(), msg,
         absl::FormatDuration(now - last).c_str());
  last = now;
}

namespace jinduo {
namespace net {
namespace details {

int createTimerfd();
void readTimerfd(int timerfd, absl::Time now);

}  // namespace details
}  // namespace net
}  // namespace jinduo

// Use relative time, immunized to wall clock changes.
class PeriodicTimer {
 public:
  PeriodicTimer(jinduo::net::EventLoop* loop, double interval,
                jinduo::net::TimerCallback cb)
      : loop_(loop),
        timerfd_(jinduo::net::details::createTimerfd()),
        timerfdChannel_(loop, timerfd_),
        interval_(interval),
        cb_(std::move(cb)) {
    timerfdChannel_.setReadCallback(
        std::bind(  // NOLINT(modernize-avoid-bind):
                    // By-design ignore timestamp arg
            &PeriodicTimer::handleRead, this));
    timerfdChannel_.enableReading();
  }

  // Disallow copy.
  PeriodicTimer(const PeriodicTimer&) noexcept = delete;
  PeriodicTimer& operator=(const PeriodicTimer&) noexcept = delete;

  // Allow move but not implemented yet.
  PeriodicTimer(PeriodicTimer&&) noexcept = delete;
  PeriodicTimer& operator=(PeriodicTimer&&) noexcept = delete;

  void start() const {
    itimerspec spec{};
    ::memset(&spec, 0, sizeof spec);
    spec.it_interval = toTimeSpec(interval_);
    spec.it_value = spec.it_interval;
    int ret =
        ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, nullptr);
    if (ret != 0) {
      LOG(ERROR) << "timerfd_settime()";
    }
  }

  ~PeriodicTimer() {
    timerfdChannel_.disableAll();
    timerfdChannel_.remove();
    ::close(timerfd_);
  }

 private:
  void handleRead() {
    loop_->assertInLoopThread();
    jinduo::net::details::readTimerfd(timerfd_, absl::Now());
    if (cb_) {
      cb_();
    }
  }

  static struct timespec toTimeSpec(double seconds) {
    timespec ts{};
    ::memset(&ts, 0, sizeof ts);
    const int64_t kNanoSecondsPerSecond = 1000000000;
    const int kMinInterval = 100000;
    auto nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    if (nanoseconds < kMinInterval) {
      nanoseconds = kMinInterval;
    }
    ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
    ts.tv_nsec =
        static_cast<decltype(ts.tv_nsec)>(nanoseconds % kNanoSecondsPerSecond);
    return ts;
  }

  jinduo::net::EventLoop* loop_;
  const int timerfd_;
  jinduo::net::Channel timerfdChannel_;
  const double interval_;  // in seconds
  jinduo::net::TimerCallback cb_;
};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  LOG(INFO) << "pid = " << getpid() << ", tid = " << jinduo::this_thread::tid()
            << " Try adjusting the wall clock, see what happens.";

  jinduo::net::EventLoop loop;
  PeriodicTimer timer(&loop, 1, [] { return print("PeriodicTimer"); });
  timer.start();
  loop.runEvery(absl::Seconds(1), [] { return print("EventLoop::runEvery"); });
  loop.loop();
}
