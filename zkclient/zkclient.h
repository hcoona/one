#pragma once

#include <functional>
#include <future>
#include <string>
#include <tuple>
#include "gsl/gsl"
#include "zkclient/zkadapter.h"

namespace hcoona {
namespace zookeeper {

class Client {
 public:
  Client();
  explicit Client(string_view host);
  Client(const Client&) = delete;
  Client& operator=(const Client&) = delete;
  Client(Client&&);
  Client& operator=(Client&& other);
  virtual ~Client();

  static void SetDebugLevel(LogLevel log_level);
  static void EnableDeterministicConnectOrder(bool yes_or_no);

  // Milliseconds according to calculate_interval in zookeeper.c
  int receive_timeout_ms() const;
  State state() const;

  ErrorCode CreateSync(string_view path, gsl::span<const gsl::byte> value,
                       gsl::span<Acl> acls, CreateFlag flags,
                       std::string* created_path);

  ErrorCode CreateAsync(string_view path, gsl::span<const gsl::byte> value,
                        gsl::span<Acl> acls, CreateFlag flags,
                        std::function<void(ErrorCode, string_view)> callback);

  std::future<std::tuple<ErrorCode, std::string>> CreateAsync(
      string_view path, gsl::span<const gsl::byte> value, gsl::span<Acl> acls,
      CreateFlag flags);

  ErrorCode DeleteSync(string_view path, int version);

  ErrorCode DeleteAsync(string_view path, int version,
                        std::function<void(ErrorCode)> callback);

  std::future<ErrorCode> DeleteAsync(string_view path, int version);

  ErrorCode ExistsSync(string_view path, Stat* stat);

 private:
  void Close();
  void Callback(WatchEventType type, State state, string_view path);

  void* handle_;
};

}  // namespace zookeeper
}  // namespace hcoona
