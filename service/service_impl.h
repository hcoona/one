#ifndef SERVICE_SERVICE_IMPL_H_
#define SERVICE_SERVICE_IMPL_H_

#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "service/service.h"
#include "service/service_state.h"

namespace hcoona {

class ServiceImpl : public Service {
 public:
  explicit ServiceImpl(std::string name);
  ~ServiceImpl() override = default;

  // Disallow copy
  ServiceImpl(const ServiceImpl&) = delete;
  ServiceImpl& operator=(const ServiceImpl&) = delete;

  // Allow move
  ServiceImpl(ServiceImpl&& other);
  ServiceImpl& operator=(ServiceImpl&& other);

  absl::Status Init(absl::any data) final;
  virtual absl::Status DoInit(absl::any data) = 0;

  absl::Status Start() final;
  virtual absl::Status DoStart() = 0;

  absl::Status Stop() final;
  virtual absl::Status DoStop() = 0;

  std::string name() const final;
  ServiceState state() const final;
  bool is_in_state(ServiceState state) const final;
  absl::Time start_time() const final;

  bool WaitForStateWithTimeout(ServiceState state,
                               absl::Duration timeout) const final;
  bool WaitForStateWithDeadline(ServiceState state,
                                absl::Time deadline) const final;

 private:
  absl::StatusOr<ServiceState> EnterState(ServiceState new_state)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  absl::Status StopInLock() ABSL_EXCLUSIVE_LOCKS_REQUIRED(state_mutex_);

  std::string name_;

  ServiceState state_ ABSL_GUARDED_BY(state_mutex_);
  absl::Time start_time_ ABSL_GUARDED_BY(state_mutex_);
  mutable absl::Mutex state_mutex_;
};

}  // namespace hcoona

#endif  // SERVICE_SERVICE_IMPL_H_
