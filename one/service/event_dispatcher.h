#ifndef SERVICE_EVENT_DISPATCHER_H_
#define SERVICE_EVENT_DISPATCHER_H_

#include <functional>
#include <memory>
#include <type_traits>

#include "absl/meta/type_traits.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "one/service/event.h"
#include "one/service/event_handler.h"

namespace hcoona {

template <typename... TEvent>
class EventDispatcher {
 public:
  using AnyEventHandlerT =
      absl::variant<std::weak_ptr<EventHandler<TEvent>>...>;
  using AnyEventT = absl::variant<TEvent...>;

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

  // TODO(zhangshuai.ustc): Support deregister handlers.
};

}  // namespace hcoona

#endif  // SERVICE_EVENT_DISPATCHER_H_
