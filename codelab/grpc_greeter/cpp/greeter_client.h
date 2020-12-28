#pragma once

#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "codelab/grpc_greeter/proto/hello.grpc.pb.h"
#include "codelab/grpc_greeter/proto/hello.pb.h"

namespace codelab {

class GreeterClient {
 public:
  explicit GreeterClient(Greeter::StubInterface* stub);
  // Assembles the client's payload, sends it and presents the
  // response back from the server.
  std::string SayHello(const std::string& user);

 private:
  Greeter::StubInterface* stub_;  // Not owned
};

}  // namespace codelab
