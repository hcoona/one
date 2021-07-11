#include <memory>
#include <string>

#include "third_party/absl/status/status.h"
#include "third_party/absl/strings/string_view.h"
#include "third_party/gflags/gflags.h"
#include "third_party/glog/logging.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/posix_file_system.h"

namespace {

constexpr const char kInputFile[] =
    "com_github_hcoona_one/codelab/read_bazel_runfiles/lipsum.txt";

}  // namespace

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::string error_message;
  std::unique_ptr<bazel::tools::cpp::runfiles::Runfiles> runfiles(
      bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error_message));

  if (!runfiles) {
    LOG(FATAL) << "Failed to create Bazel runfiles context: " << error_message;
    return 1;
  }

  gtl::PosixFileSystem file_system;
  std::unique_ptr<gtl::ReadOnlyMemoryRegion> memory_region;
  absl::Status s = file_system.NewReadOnlyMemoryRegionFromFile(
      runfiles->Rlocation(kInputFile), &memory_region);
  if (!s.ok()) {
    LOG(FATAL) << "Failed to open file '" << runfiles->Rlocation(kInputFile)
               << "': " << s.ToString();
    return 2;
  }

  LOG(INFO) << absl::string_view(
      reinterpret_cast<const char*>(memory_region->data()),
      memory_region->length());

  return 0;
}
