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
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "one/jinduo/base/thread.h"

#include <stdio.h>
#include <unistd.h>

#include <string>

#include "absl/functional/bind_front.h"
#include "one/jinduo/base/this_thread.h"

void mysleep(int seconds) {
  timespec t = {seconds, 0};
  nanosleep(&t, nullptr);
}

void threadFunc() { printf("tid=%d\n", jinduo::this_thread::tid()); }

void threadFunc2(int x) {
  printf("tid=%d, x=%d\n", jinduo::this_thread::tid(), x);
}

void threadFunc3() {
  printf("tid=%d\n", jinduo::this_thread::tid());
  mysleep(1);
}

class Foo {
 public:
  explicit Foo(double x) : x_(x) {}

  void memberFunc() const {
    printf("tid=%d, Foo::x_=%f\n", jinduo::this_thread::tid(), x_);
  }

  void memberFunc2(const std::string& text) const {
    printf("tid=%d, Foo::x_=%f, text=%s\n", jinduo::this_thread::tid(), x_,
           text.c_str());
  }

 private:
  double x_;
};

int main() {
  printf("pid=%d, tid=%d\n", ::getpid(), jinduo::this_thread::tid());

  jinduo::Thread t1(threadFunc);
  t1.start();
  printf("t1.tid=%d\n", t1.tid());
  t1.join();

  jinduo::Thread t2(absl::bind_front(threadFunc2, 42),
                    "thread for free function with argument");
  t2.start();
  printf("t2.tid=%d\n", t2.tid());
  t2.join();

  Foo foo(87.53);
  jinduo::Thread t3(absl::bind_front(&Foo::memberFunc, &foo),
                    "thread for member function without argument");
  t3.start();
  t3.join();

  jinduo::Thread t4([&foo] { foo.memberFunc2(std::string("Shuo Chen")); });
  t4.start();
  t4.join();

  {
    jinduo::Thread t5(threadFunc3);
    t5.start();
    // t5 may destruct eariler than thread creation.
  }
  mysleep(2);
  {
    jinduo::Thread t6(threadFunc3);
    t6.start();
    mysleep(2);
    // t6 destruct later than thread creation.
  }
  sleep(2);
  printf("number of created threads %d\n", jinduo::Thread::numCreated());
}
