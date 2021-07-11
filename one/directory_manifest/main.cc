#include <string>

#include "gflags/gflags.h"
#include "third_party/glog/logging.h"
#include "third_party/absl/status/status.h"
#include "gtl/file_system.h"
#include "gtl/posix_file_system.h"
#include "one/directory_manifest/generator.h"
#include "one/directory_manifest/macros.h"

namespace {

bool StringFlagNotEmpty(const char* flag_name, const std::string& value) {
  ignore_result(flag_name);
  return !value.empty();
}

}  // namespace

DEFINE_string(directory, "",
              "The directory you want to generate the manifest file.");
DEFINE_validator(directory, &StringFlagNotEmpty);

DEFINE_string(output, "manifest.csv", "The generated manifest file.");
DEFINE_validator(output, &StringFlagNotEmpty);

DEFINE_bool(overwrite, false, "Overwrite output manifest file.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  gtl::LocalPosixFileSystem file_system;
  hcoona::Generator generator(&file_system, FLAGS_directory, FLAGS_output,
                              FLAGS_overwrite);
  absl::Status s = generator.Init();
  if (!s.ok()) {
    LOG(ERROR) << s.ToString();
    return 1;
  }

  s = generator.Run();
  if (!s.ok()) {
    LOG(ERROR) << s.ToString();
    return 2;
  }

  LOG(INFO) << "Succeeded generate manifest file '" << FLAGS_output << "' for '"
            << FLAGS_directory << "'.";
  return 0;
}
