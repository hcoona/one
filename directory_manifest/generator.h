#ifndef DIRECTORY_MANIFEST_GENERATOR_H_
#define DIRECTORY_MANIFEST_GENERATOR_H_

#include <memory>
#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/synchronization/mutex.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "directory_manifest/macros.h"
#include "gtl/file_system.h"

namespace hcoona {

class Generator {
 public:
  explicit Generator(gtl::FileSystem* file_system, std::string root_directory,
                     std::string output_file, bool overwrite)
      : file_system_(file_system),
        root_directory_(std::move(root_directory)),
        output_file_(std::move(output_file)),
        overwrite_(overwrite) {}

  absl::Status Init();

  absl::Status Run();

 private:
  absl::Status Visit(std::string root);

  gtl::FileSystem* file_system_;
  std::string root_directory_;
  std::string output_file_;
  bool overwrite_;

  std::unique_ptr<gtl::WritableFile> output_writable_file_
      ABSL_GUARDED_BY(mutex_);
  mutable absl::Mutex mutex_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(Generator);
};

}  // namespace hcoona

#endif  // DIRECTORY_MANIFEST_GENERATOR_H_
