#include "service/service_state.h"

#include <type_traits>

#include "glog/logging.h"

namespace hcoona {

std::string ServiceState_Name(ServiceState service_state) {
  switch (service_state) {
    case ServiceState::kCreated:
      return "CREATED";
    case ServiceState::kInitialized:
      return "INITIALIZED";
    case ServiceState::kStarted:
      return "STARTED";
    case ServiceState::kStopped:
      return "STOPPED";
    default:
#if defined(DCHECK_IS_ON) && DCHECK_IS_ON()
      DCHECK(false) << "Unknown service_state="
                    << static_cast<std::underlying_type<ServiceState>::type>(
                           service_state);
#else
      return "UNKNOWN";
#endif
  }
}

namespace {

static_assert(
    kServiceStateSize == 4,
    "You must adjust the transition table when adding new service states.");
inline static constexpr bool
    kTransitionAllowTable[kServiceStateSize][kServiceStateSize] = {
        /*              kCreated, kInitialized, kStarted, kStopped */
        /*     kCreated */ {false, true, false, true},
        /* kInitialized */ {false, true, true, true},
        /*     kStarted */ {false, false, true, true},
        /*     kStopped */ {false, false, false, false},
};

}  // namespace

bool ValidateTransition(ServiceState current, ServiceState proposed) {
  return kTransitionAllowTable[static_cast<size_t>(current)]
                              [static_cast<size_t>(proposed)];
}

}  // namespace hcoona
