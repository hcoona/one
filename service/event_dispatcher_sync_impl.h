#ifndef SERVICE_EVENT_DISPATCHER_SYNC_IMPL_H_
#define SERVICE_EVENT_DISPATCHER_SYNC_IMPL_H_

#include <memory>
#include <tuple>
#include <type_traits>

#include "absl/status/status.h"
#include "service/event_dispatcher.h"

namespace hcoona {

template <typename... T>
class EventDispatcherSyncImpl : public EventDispatcher<T...> {
 public:
  using typename EventDispatcher<T...>::AnyEventT;
  using typename EventDispatcher<T...>::AnyEventHandlerT;

  EventDispatcherSyncImpl() = default;
  virtual ~EventDispatcherSyncImpl() = default;

  // Disallow copy.
  EventDispatcherSyncImpl(const EventDispatcherSyncImpl&) = delete;
  EventDispatcherSyncImpl& operator=(const EventDispatcherSyncImpl&) = delete;

  // Allow move.
  EventDispatcherSyncImpl(EventDispatcherSyncImpl&&) = default;
  EventDispatcherSyncImpl& operator=(EventDispatcherSyncImpl&&) = default;

  // TODO(zhangshuai.ustc): Whether to allow report error in handling?
  // I think disallow is better, the error could be reported with an error
  // event.
  absl::Status Handle(AnyEventT event) override;

  // TODO(zhangshuai.ustc): Whether to allow multi-broadcast.
  // I think it better to support multi-broadcast if we disallow report error in
  // handling.
  absl::Status RegisterHandler(AnyEventHandlerT event_handler) override;

 private:
  std::tuple<std::weak_ptr<EventHandler<T>>...> handlers_;
};

template <typename... T>
absl::Status EventDispatcherSyncImpl<T...>::Handle(
    typename EventDispatcherSyncImpl<T...>::AnyEventT event) {
  return absl::visit(
      [this](auto&& ev) -> absl::Status {
        auto handler = std::get<std::weak_ptr<EventHandler<
            typename std::decay<decltype(ev)>::type::EventTypeType>>>(handlers_)
                           .lock();
        if (handler) {
          return handler->Handle(ev);
        }

        return absl::NotFoundError("Not found handler for given event.");
      },
      event);
}

template <typename... T>
absl::Status EventDispatcherSyncImpl<T...>::RegisterHandler(
    typename EventDispatcherSyncImpl<T...>::AnyEventHandlerT event_handler) {
  return absl::visit(
      [this](auto&& h) -> absl::Status {
        auto& prev_handler =
            std::get<typename std::decay<decltype(h)>::type>(handlers_);
        if (!prev_handler.expired()) {
          return absl::AlreadyExistsError(
              "Already exists handler for given event.");
        }

        prev_handler = h;
        return absl::OkStatus();
      },
      event_handler);
}

}  // namespace hcoona

#endif  // SERVICE_EVENT_DISPATCHER_SYNC_IMPL_H_
