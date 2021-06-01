#ifndef CODELAB_MEMTABLE_MEMTABLE_H_
#define CODELAB_MEMTABLE_MEMTABLE_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "third_party/absl/status/status.h"
#include "third_party/absl/strings/string_view.h"
#include "third_party/arrow/src/arrow/io/file.h"
#include "third_party/arrow/src/parquet/api/writer.h"

namespace hcoona {
namespace codelab {

struct MemTableOptions {};

struct MemTableRow {
  MemTableRow(int64_t offset, int64_t timestamp, absl::string_view value)
      : offset(offset), timestamp(timestamp), value(std::move(value)) {}

  int64_t offset;
  int64_t timestamp;
  absl::string_view value;
};

struct BatchedResult {
  absl::Status status;
  std::vector<absl::Status> results;
};

class MemTable {
 public:
  static constexpr const int64_t kInvalidOffset = -1;

  explicit MemTable(MemTableOptions options)
      : options_(std::move(options)), latest_timestamp_(0) {}
  ~MemTable() = default;

  int64_t first_offset() const;

  int64_t last_offset() const;

  int64_t latest_timestamp() const { return latest_timestamp_; }

  // Won't append following rows if a row failed to parse or append.
  BatchedResult BatchAppendRow(const std::vector<MemTableRow>& rows);

  Status FlushToOutputStream(
      CompressType compress_type, int64_t drop_since_offset,
      std::shared_ptr<arrow::io::OutputStream> output_stream);

  Status FlushToOutputStream(
      CompressType compress_type,
      std::shared_ptr<arrow::io::OutputStream> output_stream) {
    return FlushToOutputStream(compress_type, kInvalidOffset, output_stream);
  }

  const std::vector<std::shared_ptr<SourceRow>>& GetRows() const {
    return rows_;
  }

 private:
  MemTableOptions options_;

  std::unique_ptr<arrow::RecordBatchBuilder> record_batch_builder_;
  size_t record_batch_builder_size_;

  std::vector<std::shared_ptr<arrow::RecordBatch>> record_batches_;

  int64_t latest_timestamp_;
};

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_MEMTABLE_MEMTABLE_H_
