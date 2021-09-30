#include "one/service/service_impl.h"

#include <tuple>
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/time/clock.h"
#include "glog/logging.h"

namespace hcoona {

ServiceImpl::ServiceImpl(
    std::string name,
    EventDispatcher<ServiceStateChangedEvent>* state_changed_event_dispatcher)
    : name_(std::move(name)),
      state_(ServiceState::kCreated),
      start_time_(absl::InfinitePast()),
      state_mutex_(),
      service_state_changed_event_dispatcher_(state_changed_event_dispatcher) {}

absl::Status ServiceImpl::Init(absl::any data) {
  if (is_in_state(ServiceState::kInitialized)) {
    return absl::OkStatus();
  }

  absl::WriterMutexLock state_mutex_lock(&state_mutex_);

  auto status_or_state = EnterState(ServiceState::kInitialized);
  if (!status_or_state.ok()) {
    return status_or_state.status();
  }

  // Double-checked locking
  if (status_or_state.value() != ServiceState::kInitialized) {
    absl::Status s = DoInit(std::move(data));
    if (!s.ok()) {
      // TODO(zhangshuai.ustc): Notify failure listeners & record failure if not
      // ever occurred.

      // Force stopping in case of leaking
      absl::Status stop_state = StopInLock();
      if (!stop_state.ok()) {
        LOG(WARNING) << "Failed to stop " << name_ << " : " << stop_state;
      }

      return s;
    }

    ReportServiceStatusChanging();
  }

  return absl::OkStatus();
}

absl::Status ServiceImpl::Start() {
  if (is_in_state(ServiceState::kStarted)) {
    return absl::OkStatus();
  }

  absl::WriterMutexLock state_mutex_lock(&state_mutex_);

  auto status_or_state = EnterState(ServiceState::kStarted);
  if (!status_or_state.ok()) {
    return status_or_state.status();
  }

  // Double-checked locking
  if (status_or_state.value() != ServiceState::kStarted) {
    start_time_ = absl::Now();
    absl::Status s = DoStart();
    if (!s.ok()) {
      // TODO(zhangshuai.ustc): Notify failure listeners & record failure if not
      // ever occurred.

      // Force stopping in case of leaking
      absl::Status stop_state = StopInLock();
      if (!stop_state.ok()) {
        LOG(WARNING) << "Failed to stop " << name_ << " : " << stop_state;
      }

      return s;
    }

    ReportServiceStatusChanging();
  }

  return absl::OkStatus();
}

absl::Status ServiceImpl::Stop() {
  if (is_in_state(ServiceState::kStopped)) {
    return absl::OkStatus();
  }

  absl::WriterMutexLock state_mutex_lock(&state_mutex_);

  return StopInLock();
}

absl::Status ServiceImpl::RegisterStateChangeListener(
    std::weak_ptr<EventHandler<ServiceStateChangedEvent>> handler) {
  if (!service_state_changed_event_dispatcher_) {
    return absl::FailedPreconditionError(
        "Service disallow to register any state change listener");
  }

  return service_state_changed_event_dispatcher_->RegisterHandler(handler);
}

absl::Status ServiceImpl::StopInLock() {
  auto status_or_state = EnterState(ServiceState::kStopped);
  if (!status_or_state.ok()) {
    return status_or_state.status();
  }

  // Double-checked locking
  if (status_or_state.value() != ServiceState::kStopped) {
    absl::Status s = DoStop();
    if (!s.ok()) {
      // TODO(zhangshuai.ustc): Notify failure listeners & record failure if not
      // ever occurred.
      return s;
    }

    ReportServiceStatusChanging();
  }

  return absl::OkStatus();
}

void ServiceImpl::ReportServiceStatusChanging() {
  if (service_state_changed_event_dispatcher_) {
    absl::Status s = service_state_changed_event_dispatcher_->Handle(
        ServiceStateChangedEvent(state_));
    if (!s.ok()) {
      LOG(WARNING) << "Failed to notify state changing. service=" << name_
                   << ", state=" << state_ << ", reason=" << s;
    }
  }
}

std::string ServiceImpl::name() const { return name_; }

ServiceState ServiceImpl::state() const {
  absl::ReaderMutexLock lock(&state_mutex_);
  return state_;
}

bool ServiceImpl::is_in_state(ServiceState state) const {
  absl::ReaderMutexLock lock(&state_mutex_);
  return state_ == state;
}

absl::Time ServiceImpl::start_time() const {
  absl::ReaderMutexLock lock(&state_mutex_);
  return start_time_;
}

bool ServiceImpl::WaitForStateWithTimeout(ServiceState state,
                                          absl::Duration timeout) const {
  absl::MutexLock lock(&state_mutex_);
  auto condition_data = std::make_tuple(&state_, state);
  return state_mutex_.AwaitWithTimeout(
      absl::Condition(
          +[](std::tuple<const ServiceState*, ServiceState>* data) {
            return *std::get<0>(*data) == std::get<1>(*data);
          },
          &condition_data),
      timeout);
}

bool ServiceImpl::WaitForStateWithDeadline(ServiceState state,
                                           absl::Time deadline) const {
  absl::MutexLock lock(&state_mutex_);
  auto condition_data = std::make_tuple(&state_, state);
  return state_mutex_.AwaitWithDeadline(
      absl::Condition(
          +[](std::tuple<const ServiceState*, ServiceState>* data) {
            return *std::get<0>(*data) == std::get<1>(*data);
          },
          &condition_data),
      deadline);
}

absl::StatusOr<ServiceState> ServiceImpl::EnterState(ServiceState new_state) {
  if (ValidateTransition(state_, new_state)) {
    ServiceState old_state = state_;
    state_ = new_state;
    return old_state;
  } else {
    return absl::FailedPreconditionError(absl::StrCat(
        "\"", name_, "\" cannot transite state from ",
        ServiceState_Name(state_), " to ", ServiceState_Name(new_state)));
  }
}

}  // namespace hcoona
