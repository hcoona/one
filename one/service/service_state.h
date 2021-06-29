#ifndef SERVICE_SERVICE_STATE_H_
#define SERVICE_SERVICE_STATE_H_

#include <iosfwd>
#include <string>

namespace hcoona {

// The state of a service.
//
// ATTENTION: ServiceState must increase from 0 densely.
enum class ServiceState {
  kCreated = 0,
  kInitialized = 1,
  kStarted = 2,
  kStopped = 3,
};

inline static constexpr size_t kServiceStateSize = 4;

std::string ServiceState_Name(ServiceState service_state);

inline std::ostream& operator<<(std::ostream& os, ServiceState service_state) {
  return os << ServiceState_Name(service_state);
}

bool ValidateTransition(ServiceState current, ServiceState proposed);

}  // namespace hcoona

#endif  // SERVICE_SERVICE_STATE_H_
