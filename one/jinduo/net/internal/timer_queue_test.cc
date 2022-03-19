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

#include <stdio.h>
#include <unistd.h>

#include "absl/functional/bind_front.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/jinduo/base/this_thread.h"
#include "one/jinduo/net/event_loop.h"
#include "one/jinduo/net/event_loop_thread.h"

using jinduo::net::EventLoop;
using jinduo::net::EventLoopThread;
using jinduo::net::TimerId;

int cnt = 0;
EventLoop* g_loop;

void printTid() {
  LOG(INFO) << "pid=" << ::getpid() << ", tid=" << jinduo::this_thread::tid();
}

void print(const char* msg) {
  LOG(INFO) << msg;
  if (++cnt == 20) {
    g_loop->Quit();
  }
}

void cancel(TimerId timer) {
  g_loop->CancelTimer(timer);
  LOG(INFO) << "cancelled.";
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  printTid();
  sleep(1);
  {
    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.RunAfter(absl::Seconds(1), absl::bind_front(print, "once1"));
    loop.RunAfter(absl::Seconds(1.5), absl::bind_front(print, "once1.5"));
    loop.RunAfter(absl::Seconds(2.5), absl::bind_front(print, "once2.5"));
    loop.RunAfter(absl::Seconds(3.5), absl::bind_front(print, "once3.5"));
    TimerId t45 =
        loop.RunAfter(absl::Seconds(4.5), absl::bind_front(print, "once4.5"));
    loop.RunAfter(absl::Seconds(4.2), absl::bind_front(cancel, t45));
    loop.RunAfter(absl::Seconds(4.8), absl::bind_front(cancel, t45));
    loop.RunEvery(absl::Seconds(2), absl::bind_front(print, "every2"));
    TimerId t3 =
        loop.RunEvery(absl::Seconds(3), absl::bind_front(print, "every3"));
    loop.RunAfter(absl::Seconds(9.001), absl::bind_front(cancel, t3));

    loop.Loop();
    print("main loop exits");
  }
  sleep(1);
  {
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.StartLoop();
    loop->RunAfter(absl::Seconds(2), printTid);
    sleep(3);
    print("thread loop exits");
  }
}
