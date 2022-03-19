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
// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/net/event_loop.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <thread>

#include "absl/time/time.h"
#include "one/jinduo/base/this_thread.h"

jinduo::net::EventLoop* g_loop;

void callback() {
  printf("callback(): pid = %d, tid = %d\n", getpid(),
         jinduo::this_thread::tid());
  jinduo::net::EventLoop anotherLoop;
}

void ThreadFunc() {
  printf("ThreadFunc(): pid = %d, tid = %d\n", getpid(),
         jinduo::this_thread::tid());

  assert(jinduo::net::EventLoop::GetCurrentThreadEventLoop() == nullptr);
  jinduo::net::EventLoop loop;
  assert(jinduo::net::EventLoop::GetCurrentThreadEventLoop() == &loop);
  loop.RunAfter(absl::Seconds(1), callback);
  loop.Loop();
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), jinduo::this_thread::tid());

  assert(jinduo::net::EventLoop::GetCurrentThreadEventLoop() == nullptr);
  jinduo::net::EventLoop loop;
  assert(jinduo::net::EventLoop::GetCurrentThreadEventLoop() == &loop);

  std::thread thread(ThreadFunc);

  loop.Loop();

  thread.join();
}
