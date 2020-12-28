#ifndef SERVICE_EVENT_DISPATCHER_H_
#define SERVICE_EVENT_DISPATCHER_H_

#include <memory>

#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "service/event.h"
#include "service/event_handler.h"

namespace hcoona {

template <typename... T>
class EventDispatcher {
 public:
  using AnyEventHandlerT = absl::variant<std::weak_ptr<EventHandler<T>>...>;
  using AnyEventT = absl::variant<Event<T>...>;

  EventDispatcher() = default;
  virtual ~EventDispatcher() = default;

  // Disallow copy.
  EventDispatcher(const EventDispatcher&) = delete;
  EventDispatcher& operator=(const EventDispatcher&) = delete;

  // Allow move.
  EventDispatcher(EventDispatcher&&) = default;
  EventDispatcher& operator=(EventDispatcher&&) = default;

  virtual absl::Status Handle(AnyEventT event) = 0;
  virtual absl::Status RegisterHandler(AnyEventHandlerT event_handler) = 0;
};

}  // namespace hcoona

#endif  // SERVICE_EVENT_DISPATCHER_H_
