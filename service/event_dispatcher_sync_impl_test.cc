#include "service/event_dispatcher_sync_impl.h"

#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "service/event.h"
#include "service/event_handler.h"

namespace {

enum class BasicEventType { EventOne = 1 };

std::string BasicEventType_Name(BasicEventType basic_event_type) {
  switch (basic_event_type) {
    case BasicEventType::EventOne:
      return "EVENT_ONE";
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

  // Disallow copy
  BasicEventOneEvent(const BasicEventOneEvent&) = delete;
  BasicEventOneEvent& operator=(const BasicEventOneEvent&) = delete;
};

class BasicEventHandler : public hcoona::EventHandler<BasicEventType> {
 public:
  explicit BasicEventHandler(
      std::function<absl::Status(const hcoona::Event<BasicEventType>&)>
          real_handler)
      : hcoona::EventHandler<BasicEventType>(), real_handler_(real_handler) {}

  // Disallow copy
  BasicEventHandler(const BasicEventHandler&) = delete;
  BasicEventHandler& operator=(const BasicEventHandler&) = delete;

  absl::Status Handle(const hcoona::Event<BasicEventType>& event) override;

 private:
  std::function<absl::Status(const hcoona::Event<BasicEventType>&)>
      real_handler_;
};

absl::Status BasicEventHandler::Handle(
    const hcoona::Event<BasicEventType>& event) {
  return real_handler_(event);
}

TEST(TestEventDispatcherSyncImpl, SmokeTest01) {
  absl::Time handled_time = absl::InfinitePast();

  std::shared_ptr<hcoona::EventHandler<BasicEventType>> handler =
      std::make_shared<BasicEventHandler>(
          [&handled_time](const hcoona::Event<BasicEventType>& event) {
            LOG(INFO) << "Handling event " << event;

            if (event.event_type() == BasicEventType::EventOne) {
              handled_time = absl::Now();
              return absl::OkStatus();
            }

            return absl::UnknownError("Unknown event type");
          });
  hcoona::EventDispatcherSyncImpl<BasicEventType> dispatcher;

  absl::Status s = dispatcher.RegisterHandler(
      std::weak_ptr<hcoona::EventHandler<BasicEventType>>(handler));
  ASSERT_TRUE(s.ok()) << s;

  absl::Time before_handle_time = absl::Now();
  s = dispatcher.Handle(BasicEventOneEvent());
  absl::Time after_handle_time = absl::Now();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_LT(before_handle_time, handled_time);
  EXPECT_LT(handled_time, after_handle_time);
}

}  // namespace
