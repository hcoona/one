#include "service/service_impl.h"

#include <thread>

#include "absl/synchronization/notification.h"
#include "gtest/gtest.h"

namespace {

class NaiveServiceImpl : public hcoona::ServiceImpl {
 public:
  NaiveServiceImpl();

  absl::Status DoInit(absl::any data) override;
  absl::Status DoStart() override;
  absl::Status DoStop() override;
};

NaiveServiceImpl::NaiveServiceImpl()
    : hcoona::ServiceImpl("NaiveServiceImpl") {}

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

}  // namespace
