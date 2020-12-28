#include "service/event_dispatcher_sync_impl.h"

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "service/event.h"
#include "service/event_handler.h"

namespace {

enum class BasicEventType { EventOne = 1, EventTwo = 2 };

std::string BasicEventType_Name(BasicEventType basic_event_type) {
  switch (basic_event_type) {
    case BasicEventType::EventOne:
      return "EVENT_ONE";
    case BasicEventType::EventTwo:
      return "EVENT_TWO";
    default:
#if defined(DCHECK_IS_ON) && DCHECK_IS_ON()
      DCHECK(false) << "Unknown BasicEventType="
                    << static_cast<std::underlying_type<BasicEventType>::type>(
                           basic_event_type);
#else
      return "UNKNOWN";
#endif
  }
}

std::ostream& operator<<(std::ostream& os, BasicEventType basic_event_type) {
  return os << BasicEventType_Name(basic_event_type);
}

class BasicEventOneEvent : public hcoona::Event<BasicEventType> {
 public:
  BasicEventOneEvent()
      : hcoona::Event<BasicEventType>(BasicEventType::EventOne) {}
};

static_assert(std::is_move_constructible<BasicEventOneEvent>::value,
              "BasicEventOneEvent is not move constructible.");
static_assert(std::is_move_assignable<BasicEventOneEvent>::value,
              "BasicEventOneEvent is not move assignable.");

class BasicEventTwoEvent : public hcoona::Event<BasicEventType> {
 public:
  BasicEventTwoEvent()
      : hcoona::Event<BasicEventType>(BasicEventType::EventTwo) {}
};

static_assert(std::is_move_constructible<BasicEventTwoEvent>::value,
              "BasicEventTwoEvent is not move constructible.");
static_assert(std::is_move_assignable<BasicEventTwoEvent>::value,
              "BasicEventTwoEvent is not move assignable.");

class BasicEventOneEventHandler
    : public hcoona::EventHandler<BasicEventOneEvent> {
 public:
  explicit BasicEventOneEventHandler(
      std::function<absl::Status(const BasicEventOneEvent&)> real_handler)
      : hcoona::EventHandler<BasicEventOneEvent>(),
        real_handler_(real_handler) {}

  // Disallow copy
  BasicEventOneEventHandler(const BasicEventOneEventHandler&) = delete;
  BasicEventOneEventHandler& operator=(const BasicEventOneEventHandler&) =
      delete;

  absl::Status Handle(const BasicEventOneEvent& event) override;

 private:
  std::function<absl::Status(const BasicEventOneEvent&)> real_handler_;
};

absl::Status BasicEventOneEventHandler::Handle(
    const BasicEventOneEvent& event) {
  return real_handler_(event);
}

TEST(TestEventDispatcherSyncImpl, SmokeTest01) {
  absl::Time handled_time = absl::InfinitePast();

  auto handler = std::make_shared<BasicEventOneEventHandler>(
      [&handled_time](const BasicEventOneEvent& event) -> absl::Status {
        LOG(INFO) << "Handling event " << event;

        if (event.event_type() == BasicEventType::EventOne) {
          handled_time = absl::Now();
          return absl::OkStatus();
        }

        return absl::UnknownError("Unknown event type");
      });
  hcoona::EventDispatcherSyncImpl<BasicEventOneEvent> dispatcher;

  absl::Status s = dispatcher.RegisterHandler(
      std::weak_ptr<BasicEventOneEventHandler>(handler));
  ASSERT_TRUE(s.ok()) << s;

  absl::Time before_handle_time = absl::Now();
  s = dispatcher.Handle(absl::variant<BasicEventOneEvent>());
  absl::Time after_handle_time = absl::Now();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_LT(before_handle_time, handled_time);
  EXPECT_LT(handled_time, after_handle_time);
}

TEST(TestEventDispatcherSyncImpl, SmokeTest02) {
  absl::Time one_handled_time = absl::InfinitePast();
  absl::Time two_handled_time = absl::InfinitePast();

  auto handler = std::make_shared<BasicEventOneEventHandler>(
      [&one_handled_time,
       &two_handled_time](const BasicEventOneEvent& event) -> absl::Status {
        LOG(INFO) << "Handling event " << event;

        if (event.event_type() == BasicEventType::EventOne) {
          one_handled_time = absl::Now();
          return absl::OkStatus();
        } else if (event.event_type() == BasicEventType::EventTwo) {
          two_handled_time = absl::Now();
          return absl::FailedPreconditionError("Should not handle EventTwo");
        }

        return absl::UnknownError("Unknown event type");
      });
  hcoona::EventDispatcherSyncImpl<BasicEventOneEvent, BasicEventTwoEvent>
      dispatcher;

  absl::Status s = dispatcher.RegisterHandler(
      std::weak_ptr<BasicEventOneEventHandler>(handler));
  ASSERT_TRUE(s.ok()) << s;

  absl::variant<BasicEventOneEvent, BasicEventTwoEvent> event =
      BasicEventOneEvent();

  absl::Time before_handle_time = absl::Now();
  s = dispatcher.Handle(std::move(event));
  absl::Time after_handle_time = absl::Now();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_LT(before_handle_time, one_handled_time);
  EXPECT_LT(one_handled_time, after_handle_time);
  EXPECT_EQ(absl::InfinitePast(), two_handled_time);

  event = BasicEventTwoEvent();
  ASSERT_TRUE(absl::holds_alternative<BasicEventTwoEvent>(event));

  absl::Time prev_one_handled_time = one_handled_time;
  s = dispatcher.Handle(std::move(event));
  ASSERT_FALSE(s.ok()) << "No BasicEventTwoEvent registered to dispatcher";
  EXPECT_EQ(prev_one_handled_time, one_handled_time);
  EXPECT_EQ(absl::InfinitePast(), two_handled_time);
}

}  // namespace
