#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include "gsl/gsl"
#include "zkclient/zkclient.h"

using namespace hcoona::zookeeper;  // NOLINT

constexpr char kSyncPath[] = "/zkclient_test_sync";
constexpr char kAsyncPath[] = "/zkclient_test_async";
constexpr char kNodeValue[] = "test_value";

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Please provide Zookeeper hosts." << std::endl;
    return 1;
  }

  Client::SetDebugLevel(LogLevel::kDebug);
  {
    Client client(argv[1]);
    ErrorCode error_code;

    std::string created_path;
    created_path.resize(1024);
    error_code =
        client.CreateSync(kSyncPath, gsl::as_bytes(gsl::ensure_z(kNodeValue)),
                          Acl::kRead, CreateFlag::kEphemeral, &created_path);
    if (error_code != ErrorCode::kOk) {
      std::cerr << "Failed to create zknode: " << std::string(kSyncPath) << ": "
                << to_string(error_code) << std::endl;
      return 2;
    }
    std::cout << "Successfully created zknode: " << created_path << std::endl;

    std::future<std::tuple<ErrorCode, std::string>> create_future =
        client.CreateAsync(kAsyncPath, gsl::as_bytes(gsl::ensure_z(kNodeValue)),
                           Acl::kRead, CreateFlag::kEphemeral);
    std::tie(error_code, created_path) = create_future.get();
    if (error_code != ErrorCode::kOk) {
      std::cerr << "Failed to create zknode: " << std::string(kAsyncPath)
                << ": " << to_string(error_code) << std::endl;
      return 2;
    }
    std::cout << "Successfully created zknode: " << created_path << std::endl;

    Stat stat;
    error_code = client.ExistsSync(kAsyncPath, &stat);
    if (error_code == ErrorCode::kOk) {
      std::cout << "Path " << std::string(kAsyncPath)
                << ": creation_transaction_id="
                << stat.creation_transaction_id()
                << ", creation_unix_epoch_millis="
                << stat.creation_unix_epoch_millis()
                << ", version=" << stat.version()
                << ", ephemeral_owner=" << stat.ephemeral_owner()
                << ", data_length=" << stat.data_length() << std::endl;
    } else {
      std::cout << "Path " << std::string(kAsyncPath)
                << " does not exist, error_code=" << to_string(error_code)
                << std::endl;
    }

    std::future<ErrorCode> delete_future = client.DeleteAsync(kAsyncPath, -1);
    error_code = delete_future.get();
    if (error_code != ErrorCode::kOk) {
      std::cerr << "Failed to delete zknode: " << std::string(kAsyncPath)
                << ": " << to_string(error_code) << std::endl;
      return 2;
    }
    std::cout << "Successfully delete zknode: " << std::string(kAsyncPath)
              << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(5));
  }

  std::cout << "Client destroyed, exit after 2 seconds." << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(2));
  return 0;
}
