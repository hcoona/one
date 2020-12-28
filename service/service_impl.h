#ifndef SERVICE_SERVICE_IMPL_H_
#define SERVICE_SERVICE_IMPL_H_

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "service/event_dispatcher.h"
#include "service/service.h"
#include "service/service_state.h"

namespace hcoona {

class ServiceImpl : public Service {
 public:
  ServiceImpl(std::string name, EventDispatcher<ServiceStateChangedEvent>*
                                    state_changed_event_dispatcher);
  ~ServiceImpl() override = default;

  // Disallow copy
  ServiceImpl(const ServiceImpl&) = delete;
  ServiceImpl& operator=(const ServiceImpl&) = delete;

  // Allow move
  ServiceImpl(ServiceImpl&& other);
  ServiceImpl& operator=(ServiceImpl&& other);

  absl::Status Init(absl::any data) final;
  absl::Status Start() final;
  absl::Status Stop() final;

  absl::Status RegisterStateChangeListener(
      std::weak_ptr<EventHandler<ServiceStateChangedEvent>> handler) override;

  std::string name() const final;
  ServiceState state() const final;
  bool is_in_state(ServiceState state) const final;
  absl::Time start_time() const final;

  bool WaitForStateWithTimeout(ServiceState state,
                               absl::Duration timeout) const final;
  bool WaitForStateWithDeadline(ServiceState state,
                                absl::Time deadline) const final;

 protected:
  virtual absl::Status DoInit(absl::any data) = 0;
  virtual absl::Status DoStart() = 0;
  virtual absl::Status DoStop() = 0;

 private:
  absl::StatusOr<ServiceState> EnterState(ServiceState new_state)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  absl::Status StopInLock() ABSL_EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  void ReportServiceStatusChanging()
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  const std::string name_;

  ServiceState state_ ABSL_GUARDED_BY(state_mutex_);
  absl::Time start_time_ ABSL_GUARDED_BY(state_mutex_);
  mutable absl::Mutex state_mutex_;

  EventDispatcher<ServiceStateChangedEvent>*
      service_state_changed_event_dispatcher_;
};

}  // namespace hcoona

#endif  // SERVICE_SERVICE_IMPL_H_
