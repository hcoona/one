#include "service/service_impl.h"

#include <functional>
#include <thread>
#include <utility>

#include "glog/logging.h"
#include "gtest/gtest.h"
#include "third_party/absl/synchronization/notification.h"
#include "service/event_dispatcher.h"
#include "service/event_dispatcher_sync_impl.h"

namespace {

class NaiveServiceImpl : public hcoona::ServiceImpl {
 public:
  NaiveServiceImpl();
  explicit NaiveServiceImpl(
      hcoona::EventDispatcher<hcoona::ServiceStateChangedEvent>* dispatcher);

 protected:
  absl::Status DoInit(absl::any data) override;
  absl::Status DoStart() override;
  absl::Status DoStop() override;
};

NaiveServiceImpl::NaiveServiceImpl() : NaiveServiceImpl(nullptr) {}

NaiveServiceImpl::NaiveServiceImpl(
    hcoona::EventDispatcher<hcoona::ServiceStateChangedEvent>* dispatcher)
    : hcoona::ServiceImpl("NaiveServiceImpl", dispatcher) {}

absl::Status NaiveServiceImpl::DoInit(absl::any data) {
  return absl::OkStatus();
}

absl::Status NaiveServiceImpl::DoStart() { return absl::OkStatus(); }

absl::Status NaiveServiceImpl::DoStop() { return absl::OkStatus(); }

TEST(TestNaiveServiceImpl, SmokeTest01) {
  NaiveServiceImpl service;
  EXPECT_EQ("NaiveServiceImpl", service.name());
  EXPECT_EQ(hcoona::ServiceState::kCreated, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kCreated));

  absl::Status s = service.Init(nullptr);
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kInitialized, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kInitialized));

  absl::Time before_start_time = absl::Now();
  s = service.Start();
  absl::Time after_start_time = absl::Now();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kStarted, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kStarted));
  EXPECT_LT(before_start_time, service.start_time());
  EXPECT_LT(service.start_time(), after_start_time);

  s = service.Stop();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kStopped, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kStopped));
}

TEST(TestNaiveServiceImpl, SmokeTest02) {
  NaiveServiceImpl service;
  EXPECT_EQ("NaiveServiceImpl", service.name());
  EXPECT_EQ(hcoona::ServiceState::kCreated, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kCreated));

  absl::Status s = service.Init(nullptr);
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kInitialized, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kInitialized));

  s = service.Init(nullptr);
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kInitialized, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kInitialized));

  absl::Time before_start_time = absl::Now();
  s = service.Start();
  absl::Time after_start_time = absl::Now();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kStarted, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kStarted));
  EXPECT_LT(before_start_time, service.start_time());
  EXPECT_LT(service.start_time(), after_start_time);

  s = service.Start();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kStarted, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kStarted));
  EXPECT_LT(before_start_time, service.start_time());
  EXPECT_LT(service.start_time(), after_start_time);

  s = service.Stop();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kStopped, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kStopped));

  s = service.Stop();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(hcoona::ServiceState::kStopped, service.state());
  EXPECT_TRUE(service.is_in_state(hcoona::ServiceState::kStopped));
}

TEST(TestNaiveServiceImpl, TestWaitStopWithTimeout) {
  NaiveServiceImpl service;
  absl::Notification stopped(false);

  std::thread t([&stopped, &service]() {
    if (service.WaitForStateWithTimeout(hcoona::ServiceState::kStopped,
                                        absl::Seconds(5))) {
      stopped.Notify();
    }
  });

  absl::Status s = service.Init(nullptr);
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_FALSE(stopped.HasBeenNotified());

  s = service.Start();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_FALSE(stopped.HasBeenNotified());

  s = service.Stop();
  ASSERT_TRUE(s.ok()) << s;

  t.join();
  EXPECT_TRUE(stopped.HasBeenNotified());
}

class ServiceStateChangedHandler
    : public hcoona::EventHandler<hcoona::ServiceStateChangedEvent> {
 public:
  explicit ServiceStateChangedHandler(
      std::function<absl::Status(const hcoona::ServiceStateChangedEvent&)>
          real_handler)
      : hcoona::EventHandler<hcoona::ServiceStateChangedEvent>(),
        real_handler_(std::move(real_handler)) {}

  absl::Status Handle(const hcoona::ServiceStateChangedEvent& event) override {
    return real_handler_(event);
  }

 private:
  std::function<absl::Status(const hcoona::ServiceStateChangedEvent&)>
      real_handler_;
};

TEST(TestNaiveServiceImpl, TestStateChangedNotification) {
  hcoona::EventDispatcherSyncImpl<hcoona::ServiceStateChangedEvent> dispatcher;

  hcoona::ServiceState state = hcoona::ServiceState::kCreated;
  auto handler = std::make_shared<ServiceStateChangedHandler>(
      [&state](const hcoona::ServiceStateChangedEvent& event) -> absl::Status {
        LOG(INFO) << "Handling event " << event;
        state = event.event_type();
        return absl::OkStatus();
      });
  absl::Status s = dispatcher.RegisterHandler(handler);
  ASSERT_TRUE(s.ok()) << s;

  NaiveServiceImpl service(&dispatcher);
  EXPECT_EQ(service.state(), state);

  s = service.Init(nullptr);
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(service.state(), state);

  s = service.Start();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(service.state(), state);

  s = service.Stop();
  ASSERT_TRUE(s.ok()) << s;
  EXPECT_EQ(service.state(), state);
}

}  // namespace
