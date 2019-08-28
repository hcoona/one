#include "zkclient/zkclient.h"

#include <memory>
#include <utility>
#include "zookeeper.h"      // NOLINT
#include "zookeeper_log.h"  // NOLINT

namespace hcoona {
namespace zookeeper {

// TODO(hcoona): Move them into configuration class.
static constexpr int kReceiveTimeoutMs = 30000;

static zhandle_t* to_zoo(void* handle) {
  return reinterpret_cast<zhandle_t*>(handle);
}

static ACL_vector to_zoo(gsl::span<Acl> acls) {
  return {gsl::narrow_cast<int>(acls.size()),
          reinterpret_cast<::ACL*>(acls.data())};
}

static ::Stat* to_zoo(Stat* stat) { return reinterpret_cast<::Stat*>(stat); }

template <class T>
class Holder {
 public:
  T t_;
};

Client::Client() = default;

// TODO(hcoona): Persist & Reload client_id from file.
Client::Client(string_view host) {
  handle_ = zookeeper_init(
      host.data(),
      +[](zhandle_t* zh, int type, int state, const char* path,
          void* watcherCtx) {
        Client* client = reinterpret_cast<Client*>(watcherCtx);
        WatchEventType watch_type = static_cast<WatchEventType>(type);
        State zk_state = static_cast<State>(state);

        client->Callback(watch_type, zk_state, path);
      },
      kReceiveTimeoutMs, nullptr, this, 0);
}

Client::Client(Client&& other) {
  handle_ = other.handle_;
  other.handle_ = nullptr;
}

Client& Client::operator=(Client&& other) {
  if (this != &other) {
    Close();

    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

Client::~Client() { Close(); }

void Client::SetDebugLevel(LogLevel log_level) {
  zoo_set_debug_level(static_cast<ZooLogLevel>(log_level));
}

void Client::EnableDeterministicConnectOrder(bool yes_or_no) {
  zoo_deterministic_conn_order(yes_or_no);
}

int Client::receive_timeout_ms() const {
  return zoo_recv_timeout(to_zoo(handle_));
}

State Client::state() const {
  return static_cast<State>(zoo_state(to_zoo(handle_)));
}

ErrorCode Client::CreateSync(string_view path, gsl::span<const gsl::byte> value,
                             gsl::span<Acl> acls, CreateFlag flags,
                             std::string* created_path) {
  struct ACL_vector z_acls = to_zoo(acls);
  return static_cast<ErrorCode>(zoo_create(
      to_zoo(handle_), path.data(), reinterpret_cast<const char*>(value.data()),
      value.size_bytes(), &z_acls, static_cast<int>(flags),
      const_cast<char*>(created_path->data()), created_path->size()));
}

ErrorCode Client::CreateAsync(
    string_view path, gsl::span<const gsl::byte> value, gsl::span<Acl> acls,
    CreateFlag flags, std::function<void(ErrorCode, string_view)> callback) {
  auto holder = new Holder<std::function<void(ErrorCode, string_view)>>;
  holder->t_ = std::move(callback);

  struct ACL_vector z_acls = to_zoo(acls);
  ErrorCode error_code = static_cast<ErrorCode>(zoo_acreate(
      to_zoo(handle_), path.data(), reinterpret_cast<const char*>(value.data()),
      value.size_bytes(), &z_acls, static_cast<int>(flags),
      [](int rc, const char* value, const void* data) {
        auto holder =
            (Holder<std::function<void(ErrorCode, string_view)>>*)data;
        std::function<void(ErrorCode, string_view)> callback =
            std::move(holder->t_);
        delete holder;

        callback(static_cast<ErrorCode>(rc), value);
      },
      holder));
  if (error_code != ErrorCode::kOk) {
    delete holder;
  }

  return error_code;
}

std::future<std::tuple<ErrorCode, std::string>> Client::CreateAsync(
    string_view path, gsl::span<const gsl::byte> value, gsl::span<Acl> acls,
    CreateFlag flags) {
  auto promise =
      std::make_shared<std::promise<std::tuple<ErrorCode, std::string>>>();
  ErrorCode error_code =
      CreateAsync(path, value, acls, flags,
                  [promise](ErrorCode error_code, string_view created_path) {
                    promise->set_value(
                        std::make_tuple(error_code, std::string(created_path)));
                  });
  if (error_code != ErrorCode::kOk) {
    promise->set_value(std::make_tuple(error_code, std::string(path)));
  }
  return promise->get_future();
}

ErrorCode Client::DeleteSync(string_view path, int version) {
  return static_cast<ErrorCode>(
      zoo_delete(to_zoo(handle_), path.data(), version));
}

ErrorCode Client::DeleteAsync(string_view path, int version,
                              std::function<void(ErrorCode)> callback) {
  auto holder = new Holder<std::function<void(ErrorCode)>>;
  holder->t_ = std::move(callback);

  ErrorCode error_code = static_cast<ErrorCode>(zoo_adelete(
      to_zoo(handle_), path.data(), version,
      [](int rc, const void* data) {
        auto holder = (Holder<std::function<void(ErrorCode)>>*)data;
        std::function<void(ErrorCode)> callback = std::move(holder->t_);
        delete holder;

        callback(static_cast<ErrorCode>(rc));
      },
      holder));
  if (error_code != ErrorCode::kOk) {
    delete holder;
  }

  return error_code;
}

std::future<ErrorCode> Client::DeleteAsync(string_view path, int version) {
  auto promise = std::make_shared<std::promise<ErrorCode>>();
  ErrorCode error_code = DeleteAsync(
      path, version,
      [promise](ErrorCode error_code) { promise->set_value(error_code); });
  if (error_code != ErrorCode::kOk) {
    promise->set_value(error_code);
  }
  return promise->get_future();
}

ErrorCode Client::ExistsSync(string_view path, Stat* stat) {
  return static_cast<ErrorCode>(
      zoo_exists(to_zoo(handle_), path.data(), 0, to_zoo(stat)));
}

void Client::Close() {
  if (handle_) {
    ErrorCode error_code =
        static_cast<ErrorCode>(zookeeper_close(to_zoo(handle_)));
    if (error_code != ErrorCode::kOk) {
      LOG_ERROR(("[handle=%x] Failed to close zookeeper session, error_code=%s",
                 handle_, to_string(error_code).data()));
    }
  }
}

void Client::Callback(WatchEventType type, State state, string_view path) {
  LOG_DEBUG(("[handle=%x] type=%s, state=%s, path=%s", handle_,
             to_string(type).data(), to_string(state).data(), path.data()));

  if (type == WatchEventType::kSession) {
    LOG_WARN(("[handle=%x] Session state changed to %s", handle_,
              to_string(state).data()));
    return;
  }
}

}  // namespace zookeeper
}  // namespace hcoona
