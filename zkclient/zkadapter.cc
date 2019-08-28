#include "zkclient/zkadapter.h"

#include <cstring>
#include <utility>
#include "zookeeper.h"  // NOLINT

static_assert(sizeof(hcoona::zookeeper::Id) == sizeof(Id),
              "struct Id mismatch.");
static_assert(sizeof(hcoona::zookeeper::Acl) == sizeof(ACL),
              "struct ACL mismatch.");

namespace hcoona {
namespace zookeeper {

std::string to_string(LogLevel log_level) {
  switch (log_level) {
    case LogLevel::kError:
      return "ZOO_LOG_LEVEL_ERROR";
    case LogLevel::kWarning:
      return "ZOO_LOG_LEVEL_WARN";
    case LogLevel::kInformation:
      return "ZOO_LOG_LEVEL_INFO";
    case LogLevel::kDebug:
      return "ZOO_LOG_LEVEL_DEBUG";
    default:
      return "INVALID_LOG_LEVEL";
  }
}

std::string to_string(ErrorCode error_code) {
  return zerror(static_cast<int>(error_code));
}

// Defined in zookeeper.c
std::string to_string(State state) {
  switch (state) {
    case State::kClosed:
      return "ZOO_CLOSED_STATE";
    case State::kConnecting:
      return "ZOO_CONNECTING_STATE";
    case State::kAssociating:
      return "ZOO_ASSOCIATING_STATE";
    case State::kConnected:
      return "ZOO_CONNECTED_STATE";
    case State::kExpiredSession:
      return "ZOO_EXPIRED_SESSION_STATE";
    case State::kAuthFailed:
      return "ZOO_AUTH_FAILED_STATE";
    default:
      return "INVALID_STATE";
  }
}

// Defined in zookeeper.c
std::string to_string(WatchEventType event_type) {
  switch (event_type) {
    case WatchEventType::kError:
      return "ZOO_ERROR_EVENT";
    case WatchEventType::kCreated:
      return "ZOO_CREATED_EVENT";
    case WatchEventType::kDeleted:
      return "ZOO_DELETED_EVENT";
    case WatchEventType::kChanged:
      return "ZOO_CHANGED_EVENT";
    case WatchEventType::kChild:
      return "ZOO_CHILD_EVENT";
    case WatchEventType::kSession:
      return "ZOO_SESSION_EVENT";
    case WatchEventType::kNotWatching:
      return "ZOO_NOTWATCHING_EVENT";
    default:
      return "INVALID_EVENT";
  }
}

std::string to_string(CreateFlag create_flag) {
  switch (create_flag) {
    case CreateFlag::kEphemeral:
      return "ZOO_EPHEMERAL";
    case CreateFlag::kSequence:
      return "ZOO_SEQUENCE";
    default:
      return "INVALID_CREATE_FLAG";
  }
}

Id::Id() : scheme_(nullptr), id_(nullptr) {}

Id::Id(string_view scheme, string_view id) {
  scheme_ = new char[scheme.size() + 1];
  std::memcpy(scheme_, scheme.data(), scheme.size() + 1);

  id_ = new char[id.size() + 1];
  std::memcpy(id_, id.data(), id.size() + 1);
}

Id::Id(const Id& other) {
  size_t scheme_length = std::strlen(other.scheme_);
  scheme_ = new char[scheme_length + 1];
  std::memcpy(scheme_, other.scheme_, scheme_length + 1);

  size_t id_length = std::strlen(other.id_);
  id_ = new char[id_length + 1];
  std::memcpy(id_, other.id_, id_length + 1);
}

Id& Id::operator=(const Id& other) {
  if (this == &other) {
    return *this;
  }

  if (scheme_ != nullptr) {
    delete scheme_;
  }
  if (id_ != nullptr) {
    delete id_;
  }

  if (other.scheme_ == nullptr) {
    scheme_ = nullptr;
  } else {
    size_t scheme_length = std::strlen(other.scheme_);
    scheme_ = new char[scheme_length + 1];
    std::memcpy(scheme_, other.scheme_, scheme_length + 1);
  }

  if (other.id_ == nullptr) {
    id_ = nullptr;
  } else {
    size_t id_length = std::strlen(other.id_);
    id_ = new char[id_length + 1];
    std::memcpy(id_, other.id_, id_length + 1);
  }

  return *this;
}

Id::Id(Id&& other) {
  scheme_ = other.scheme_;
  other.scheme_ = nullptr;

  id_ = other.id_;
  other.id_ = nullptr;
}

Id& Id::operator=(Id&& other) {
  if (this == &other) {
    return *this;
  }

  if (scheme_ != nullptr) {
    delete scheme_;
  }
  if (id_ != nullptr) {
    delete id_;
  }

  scheme_ = other.scheme_;
  other.scheme_ = nullptr;

  id_ = other.id_;
  other.id_ = nullptr;

  return *this;
}

Id::~Id() {
  if (scheme_ != nullptr) {
    delete scheme_;
  }

  if (id_ != nullptr) {
    delete id_;
  }
}

// Keep align with ZOO_ANYONE_ID_UNSAFE
const Id Id::kAnyone{"world", "anyone"};
// Keep align with ZOO_AUTH_IDS
const Id Id::kCurrentAuthenticated{"auth", ""};

Acl::Acl(PermissionFlag perms, Id id) : perms_(perms), id_(std::move(id)) {}

// Keep align with ZOO_OPEN_ACL_UNSAFE
static Acl kOpenAclUnsafeAcl[] = {{PermissionFlag::kAll, Id::kAnyone}};
const gsl::span<Acl> Acl::kOpen = kOpenAclUnsafeAcl;
// Keep align with ZOO_READ_ACL_UNSAFE
static Acl kReadAclUnsafeAcl[] = {{PermissionFlag::kRead, Id::kAnyone}};
const gsl::span<Acl> Acl::kRead = kReadAclUnsafeAcl;
// Keep align with ZOO_CREATOR_ALL_ACL
static Acl kCreatorAllAclAcl[] = {
    {PermissionFlag::kAll, Id::kCurrentAuthenticated}};
const gsl::span<Acl> Acl::kCreatorAll = kCreatorAllAclAcl;

}  // namespace zookeeper
}  // namespace hcoona
