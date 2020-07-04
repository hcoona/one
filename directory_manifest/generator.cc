#include "directory_manifest/generator.h"

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "directory_manifest/macros.h"
#include "gtl/file_statistics.h"
#include "gtl/file_system.h"

namespace hcoona {

absl::Status Generator::Init() {
  RETURN_IF_ERROR(file_system_->IsDirectory(std::string(root_directory_)));
  absl::Status s = file_system_->FileExists(std::string(output_file_));
  if (s.ok()) {
    return absl::FailedPreconditionError(
        absl::StrCat(output_file_, " already exists."));
  } else if (!absl::IsNotFound(s)) {
    return s;
  }

  RETURN_IF_ERROR(
      file_system_->NewWritableFile(output_file_, &output_writable_file_));

  return absl::OkStatus();
}

absl::Status Generator::Run() { return Visit(root_directory_); }

absl::Status Generator::Visit(std::string root) {
  std::vector<std::string> children;
  RETURN_IF_ERROR(file_system_->GetChildren(root, &children));

  VLOG(1) << "Found " << children.size() << " files under " << root;
  for (const std::string& filename : children) {
    std::string path = file_system_->JoinPath(root, filename);

    gtl::FileStatistics stat;
    absl::Status s = file_system_->Stat(path, &stat);
    if (!s.ok()) {
      LOG(WARNING) << "Failed to get file statistics for " << path;
      continue;
    }

    LOG(INFO) << "{name=" << filename << ", path=" << path
              << ", length_bytes=" << stat.length
              << ", mtime=" << absl::FromUnixNanos(stat.mtime_nsec)
              << ", is_directory=" << stat.is_directory << "}";
  }

  return absl::OkStatus();
}

}  // namespace hcoona
