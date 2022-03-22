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

#include <any>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "absl/time/time.h"
#include "one/jinduo/net/buffer.h"
#include "one/jinduo/net/callbacks.h"
#include "one/jinduo/net/inet_address.h"

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace jinduo {
namespace net {

class Channel;
class EventLoop;
class Socket;

///
/// TCP connection, for both client and server usage.
///
/// This is an interface class, so don't expose too much details.
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  /// Constructs a TcpConnection with a connected sockfd
  ///
  /// User should not create this object.
  TcpConnection(EventLoop* loop, std::string name, int sockfd,
                const InetAddress& localAddr, const InetAddress& peerAddr);
  ~TcpConnection();

  // Disallow copy.
  TcpConnection(const TcpConnection&) noexcept = delete;
  TcpConnection& operator=(const TcpConnection&) noexcept = delete;

  // Allow move but no implemented yet.
  TcpConnection(TcpConnection&&) noexcept = delete;
  TcpConnection& operator=(TcpConnection&&) noexcept = delete;

  EventLoop* getLoop() const { return loop_; }
  const std::string& name() const { return name_; }
  const InetAddress& localAddress() const { return localAddr_; }
  const InetAddress& peerAddress() const { return peerAddr_; }
  bool connected() const { return state_ == kConnected; }
  bool disconnected() const { return state_ == kDisconnected; }
  // return true if success.
  bool getTcpInfo(struct tcp_info*) const;
  std::string getTcpInfoString() const;

  // void send(std::string&& message); // C++11
  void send(const void* message, int len);
  void send(const std::string_view& message);
  void send(Buffer* message);  // this one will swap data
  void shutdown();             // NOT thread safe, no simultaneous calling
  // void shutdownAndForceCloseAfter(double seconds); // NOT thread safe, no
  // simultaneous calling
  void forceClose();
  void forceCloseWithDelay(absl::Duration duration);
  void setTcpNoDelay(bool on);
  // reading or not
  void startRead();
  void stopRead();
  bool isReading() const {
    // NOT thread safe, may race with start/stopReadInLoop
    return reading_;
  }

  void setContext(std::any&& context) { context_ = std::move(context); }
  void setContext(const std::any& context) { context_ = context; }

  const std::any& getContext() const { return context_; }

  std::any* getMutableContext() { return &context_; }

  void setConnectionCallback(const ConnectionCallback& cb) {
    connectionCallback_ = cb;
  }

  void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    writeCompleteCallback_ = cb;
  }

  void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                size_t highWaterMark) {
    highWaterMarkCallback_ = cb;
    highWaterMark_ = highWaterMark;
  }

  /// Advanced interface
  Buffer* inputBuffer() { return &inputBuffer_; }

  Buffer* outputBuffer() { return &outputBuffer_; }

  /// Internal use only.
  void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

  // called when TcpServer accepts a new connection
  void connectEstablished();  // should be called only once
  // called when TcpServer has removed me from its map
  void connectDestroyed();  // should be called only once

 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
  void handleRead(absl::Time receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  // void sendInLoop(std::string&& message);
  void sendInLoop(const std::string_view& message);
  void sendInLoop(const void* message, size_t len);
  void shutdownInLoop();
  // void shutdownAndForceCloseInLoop(double seconds);
  void forceCloseInLoop();
  void setState(StateE s) { state_ = s; }
  const char* stateToString() const;
  void startReadInLoop();
  void stopReadInLoop();

  EventLoop* loop_;
  const std::string name_;
  StateE state_;  // FIXME: use atomic variable
  bool reading_;
  // we don't expose those classes to client.
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;
  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;
  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;  // FIXME: use list<Buffer> as output buffer.
  std::any context_;
  // FIXME: creationTime_, lastReceiveTime_
  //        bytesReceived_, bytesSent_
};

}  // namespace net
}  // namespace jinduo
