#include <cstdio>

#include "absl/debugging/failure_signal_handler.h"
#include "absl/debugging/symbolize.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "gtest/gtest.h"

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);

  google::InitGoogleLogging(argv[0]);
  google::ParseCommandLineFlags(&argc, &argv, /* remove_flags= */ true);

  absl::InitializeSymbolizer(argv[0]);

  absl::FailureSignalHandlerOptions options;
  absl::InstallFailureSignalHandler(options);

  return RUN_ALL_TESTS();
}
