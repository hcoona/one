#include "one/codelab/grpc_greeter/cpp/greeter_client.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "one/codelab/grpc_greeter/proto/hello.pb.h"
#include "one/codelab/grpc_greeter/proto/hello_mock.grpc.pb.h"

namespace codelab {
namespace {

using grpc::Status;
using testing::_;
using testing::AtLeast;
using testing::DoAll;
using testing::Return;
using testing::SetArgPointee;

class GreeterClientTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {}

  virtual void TearDown() override {}
};

TEST_F(GreeterClientTest, testHello) {
  // Setup mock stub to return a fixed reply.
  MockGreeterStub stub;
  HelloReply reply;
  reply.set_message("hello world");
  EXPECT_CALL(stub, SayHello(_, _, _))
      .Times(AtLeast(1))
      .WillOnce(DoAll(SetArgPointee<2>(reply), Return(Status::OK)));
  GreeterClient greeter(&stub);

  std::string user = "world";
  EXPECT_EQ("hello world", greeter.SayHello(user));
}

}  // namespace
}  // namespace codelab
