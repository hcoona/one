#include "one/directory_manifest/generator.h"

#include <queue>
#include <string>
#include <vector>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "third_party/absl/status/status.h"
#include "third_party/absl/strings/escaping.h"
#include "third_party/absl/strings/str_cat.h"
#include "third_party/absl/time/time.h"
#include "gtl/container/circular_deque.h"
#include "gtl/crypto/sha2.h"
#include "gtl/file_statistics.h"
#include "gtl/file_system.h"
#include "one/directory_manifest/macros.h"

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
  absl::Status s;
  CircularQueue<std::string> visit_queue;
  visit_queue.emplace(std::move(root));

  // TODO(zhangshuai.ustc): Multi-threading to boost the enumeration.
  while (!visit_queue.empty()) {
    std::string current = visit_queue.front();
    visit_queue.pop();

    std::vector<std::string> children;
    RETURN_IF_ERROR(file_system_->GetChildren(current, &children));

    VLOG(1) << "Found " << children.size() << " files under " << current;
    for (const std::string& filename : children) {
      std::string path = file_system_->JoinPath(current, filename);

      gtl::FileStatistics stat;
      s = file_system_->Stat(path, &stat);
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
      } else {
        // TODO(zhangshuai.ustc): Post task into thread pool.
        std::unique_ptr<gtl::ReadOnlyMemoryRegion> memory_region;
        s = file_system_->NewReadOnlyMemoryRegionFromFile(path, &memory_region);
        if (!s.ok()) {
          LOG(ERROR) << "Failed to read file '" << path
                     << "': " << s.ToString();
          continue;
        }

        std::array<uint8_t, gtl::crypto::kSHA256Length> sha256_bytes =
            gtl::crypto::SHA256Hash(absl::MakeSpan(
                reinterpret_cast<const uint8_t*>(memory_region->data()),
                memory_region->length()));
        std::string sha256_hex_string = absl::BytesToHexString(
            absl::string_view(reinterpret_cast<const char*>(&sha256_bytes[0]),
                              sha256_bytes.size()));
        // TODO(zhangshuai.ustc): Write to tables.
        LOG(INFO) << "\"" << path << "\",\"" << sha256_hex_string << "\"";
      }
    }
  }

  return absl::OkStatus();
}

}  // namespace hcoona
