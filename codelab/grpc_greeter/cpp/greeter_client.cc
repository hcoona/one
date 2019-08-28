#include "codelab/grpc_greeter/cpp/greeter_client.h"

#include <iostream>

namespace codelab {

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

// Constructor with "initialization list"
GreeterClient::GreeterClient(Greeter::StubInterface* stub) : stub_(stub) {}

std::string GreeterClient::SayHello(const std::string& user) {
  // Data we are sending to the server.
  HelloRequest request;
  request.set_name(user);

  // Container for the data we expect from the server.
  HelloReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->SayHello(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    return reply.message();
  } else {
    return "RPC failed";
  }
}

}  // namespace codelab
