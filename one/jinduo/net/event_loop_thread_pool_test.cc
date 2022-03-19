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

#include "one/jinduo/net/event_loop_thread_pool.h"

#include <stdio.h>
#include <unistd.h>

#include "absl/functional/bind_front.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "one/jinduo/net/event_loop.h"

using jinduo::net::EventLoop;
using jinduo::net::EventLoopThreadPool;

void print(EventLoop* p = NULL) {
  LOG(INFO) << "main(): pid=" << ::getpid()
            << ", tid=" << jinduo::this_thread::tid() << ", loop=" << p;
}

void init(EventLoop* p) {
  LOG(INFO) << "init(): pid=" << ::getpid()
            << ", tid=" << jinduo::this_thread::tid() << ", loop=" << p;
}

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  gflags::ParseCommandLineFlags(&argc, &argv, /*remove_flags=*/true);

  print();

  EventLoop loop;
  loop.RunAfter(absl::Seconds(11), absl::bind_front(&EventLoop::Quit, &loop));

  {
    LOG(INFO) << "Single thread (p=" << &loop << ")";
    EventLoopThreadPool model(&loop, "single");
    model.set_thread_num(0);
    model.Start(init);
    assert(model.GetNextLoop() == &loop);
    assert(model.GetNextLoop() == &loop);
    assert(model.GetNextLoop() == &loop);
  }

  {
    LOG(INFO) << "Another thread (p=" << &loop << ")";
    EventLoopThreadPool model(&loop, "another");
    model.set_thread_num(1);
    model.Start(init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunAfter(absl::Seconds(2), absl::bind_front(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop == model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
    ::sleep(3);
  }

  {
    LOG(INFO) << "Three threads (p=" << &loop << ")";
    EventLoopThreadPool model(&loop, "three");
    model.set_thread_num(3);
    model.Start(init);
    EventLoop* nextLoop = model.GetNextLoop();
    nextLoop->RunInLoop(absl::bind_front(print, nextLoop));
    assert(nextLoop != &loop);
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop != model.GetNextLoop());
    assert(nextLoop == model.GetNextLoop());
  }

  loop.Loop();
}
