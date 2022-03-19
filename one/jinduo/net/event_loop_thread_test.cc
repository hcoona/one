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

#include "one/jinduo/net/event_loop_thread.h"

#include <stdio.h>
#include <unistd.h>

#include "absl/functional/bind_front.h"
#include "absl/time/time.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"

using jinduo::net::EventLoop;
using jinduo::net::EventLoopThread;

void print(EventLoop* p = nullptr) {
  printf("print: pid = %d, tid = %d, loop = %p\n", getpid(),
         jinduo::this_thread::tid(), p);
}

void quit(EventLoop* p) {
  print(p);
  p->Quit();
}

int main(int /*argc*/, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  LOG(INFO) << "Hello World!";

  print();

  {
    EventLoopThread thr1{};  // never start
  }

  {
    // dtor calls quit()
    EventLoopThread thr2{};
    EventLoop* loop = thr2.StartLoop();
    loop->RunInLoop(absl::bind_front(print, loop));
    absl::SleepFor(absl::Milliseconds(500));
  }

  {
    // quit() before dtor
    EventLoopThread thr3{};
    EventLoop* loop = thr3.StartLoop();
    loop->RunInLoop(absl::bind_front(quit, loop));
    absl::SleepFor(absl::Milliseconds(500));
  }
}
