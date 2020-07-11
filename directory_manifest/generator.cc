#include "directory_manifest/generator.h"

#include <queue>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/time/time.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "directory_manifest/macros.h"
#include "gtl/container/circular_deque.h"
#include "gtl/file_statistics.h"
#include "gtl/file_system.h"

namespace hcoona {

// Provides a definition of base::queue that's like std::queue but uses a
// base::circular_deque instead of std::deque. Since std::queue is just a
// wrapper for an underlying type, we can just provide a typedef for it that
// defaults to the base circular_deque.
template <class T, class Container = gtl::circular_deque<T>>
using CircularQueue = std::queue<T, Container>;

absl::Status Generator::Init() {
  RETURN_IF_ERROR(file_system_->IsDirectory(std::string(root_directory_)));
  absl::Status s = file_system_->FileExists(std::string(output_file_));
  if (s.ok()) {
    if (overwrite_) {
      LOG(WARNING) << "The output file '" << output_file_
                   << "' exists, overwrite it.";
    } else {
      return absl::FailedPreconditionError(
          absl::StrCat(output_file_, " already exists."));
    }
  } else if (!absl::IsNotFound(s)) {
    return s;
  }

  RETURN_IF_ERROR(
      file_system_->NewWritableFile(output_file_, &output_writable_file_));

  return absl::OkStatus();
}

absl::Status Generator::Run() { return Visit(root_directory_); }

absl::Status Generator::Visit(std::string root) {
  CircularQueue<std::string> visit_queue;
  visit_queue.emplace(std::move(root));

  // TODO(zhangshuai.ustc): Multi-threading to boost it.
  while (!visit_queue.empty()) {
    std::string current = visit_queue.front();
    visit_queue.pop();

    std::vector<std::string> children;
    RETURN_IF_ERROR(file_system_->GetChildren(current, &children));

    VLOG(1) << "Found " << children.size() << " files under " << current;
    for (const std::string& filename : children) {
      std::string path = file_system_->JoinPath(current, filename);

      gtl::FileStatistics stat;
      absl::Status s = file_system_->Stat(path, &stat);
      if (!s.ok()) {
        LOG(WARNING) << "Failed to get file statistics for " << path;
        continue;
      }

      VLOG(2) << "{name=" << filename << ", path=" << path
              << ", length_bytes=" << stat.length
              << ", mtime=" << absl::FromUnixNanos(stat.mtime_nsec)
              << ", is_directory=" << stat.is_directory << "}";
      if (stat.is_directory) {
        visit_queue.emplace(std::move(path));
      }
    }
  }

  return absl::OkStatus();
}

}  // namespace hcoona
