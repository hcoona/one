#include <cstdio>

#include "gflags/gflags.h"
#include "third_party/glog/logging.h"
#include "third_party/absl/debugging/failure_signal_handler.h"
#include "third_party/absl/debugging/symbolize.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);

  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

  absl::InitializeSymbolizer(argv[0]);

  absl::FailureSignalHandlerOptions options;
  absl::InstallFailureSignalHandler(options);

  return RUN_ALL_TESTS();
}
