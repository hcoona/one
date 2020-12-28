#ifndef SERVICE_EVENT_HANDLER_H_
#define SERVICE_EVENT_HANDLER_H_

#include <type_traits>

#include "absl/status/status.h"
#include "service/event.h"

namespace hcoona {

template <typename TEvent>
class EventHandler {
 public:
  EventHandler() = default;
  virtual ~EventHandler() = default;

  // Disallow copy
  EventHandler(const EventHandler&) = delete;
  EventHandler& operator=(const EventHandler&) = delete;

  // Allow move
  EventHandler(EventHandler&&) = default;
  EventHandler& operator=(EventHandler&&) = default;

  // UNSTABLE
  // Need change for unifying sync & async.
  virtual absl::Status Handle(const TEvent& event) = 0;
};

}  // namespace hcoona

#endif  // SERVICE_EVENT_HANDLER_H_
