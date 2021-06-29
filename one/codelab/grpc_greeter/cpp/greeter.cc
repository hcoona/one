#include <iostream>
#include <memory>

#include "one/codelab/grpc_greeter/cpp/greeter_client.h"

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the
  // actual RPCs are created. This channel models a connection to an
  // endpoint (in this case, localhost at port 50051). We indicate
  // that the channel isn't authenticated (use of
  // InsecureCredentials()).
  std::unique_ptr<codelab::Greeter::StubInterface> stub(
      codelab::Greeter::NewStub(grpc::CreateChannel(
          "localhost:50051", grpc::InsecureChannelCredentials())));
  codelab::GreeterClient greeter(stub.get());
  std::string user("world");
  std::string reply = greeter.SayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}
