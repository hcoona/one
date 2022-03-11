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
//
// This is a public header file, it must only include public header files.

#pragma once

#include <functional>
#include <memory>

#include "absl/time/time.h"
#include "one/base/down_cast.h"

namespace jinduo {
namespace net {

// All client visible callbacks go here.

class Buffer;
class TcpConnection;
using TimerCallback = std::function<void()>;
using ConnectionCallback =
    std::function<void(const std::shared_ptr<TcpConnection>&)>;
using CloseCallback =
    std::function<void(const std::shared_ptr<TcpConnection>&)>;
using WriteCompleteCallback =
    std::function<void(const std::shared_ptr<TcpConnection>&)>;
using HighWaterMarkCallback =
    std::function<void(const std::shared_ptr<TcpConnection>&, size_t)>;

// the data has been read to (buf, len)
using MessageCallback = std::function<void(
    const std::shared_ptr<TcpConnection>&, Buffer*, absl::Time)>;

void defaultConnectionCallback(const std::shared_ptr<TcpConnection>& conn);
void defaultMessageCallback(const std::shared_ptr<TcpConnection>& conn,
                            Buffer* buffer, absl::Time receiveTime);

}  // namespace net
}  // namespace jinduo
