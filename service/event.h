#ifndef SERVICE_EVENT_H_
#define SERVICE_EVENT_H_

#include <iosfwd>
#include <type_traits>
#include <utility>

#include "absl/time/clock.h"
#include "absl/time/time.h"

namespace hcoona {

template <typename T>
class Event {
  static_assert(std::is_enum<T>::value, "T must be an enum for event type.");

 public:
  using EventTypeT = T;

  explicit Event(T event_type)
      : event_type_(event_type), creation_time_(absl::Now()) {}
  virtual ~Event() = default;

  // Disallow copy
  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;

  // Allow move
  Event(Event&& other)
      : event_type_(other.event_type_),
        creation_time_(std::move(other.creation_time_)) {}
  Event& operator=(Event&& other) {
    event_type_ = other.event_type_;
    creation_time_ = std::move(other.creation_time_);
    return *this;
  }

  T event_type() const { return event_type_; }

  absl::Time creation_time() const { return creation_time_; }

 private:
  T event_type_;
  absl::Time creation_time_;
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const Event<T>& event) {
  return os << event.event_type() << "@"
            << absl::FormatTime(absl::RFC3339_sec, event.creation_time(),
                                absl::UTCTimeZone());
}

}  // namespace hcoona

#endif  // SERVICE_EVENT_H_
