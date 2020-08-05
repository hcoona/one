#ifndef CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_DUMP_H_
#define CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_DUMP_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "arrow/api.h"
#include "codelab/feature_store/benchmark_encoding/row.h"

namespace hcoona {
namespace codelab {
namespace feature_store {

enum class CompressionMode : int32_t {
  kNoCompression = 0,
  kSnappy = 1,
};

enum class FieldType {
  kUnspecified = 0,
  kFeature = 1,
  kRawFeature = 2,
};

struct FieldDescriptor {
  std::string name;
  FieldType type;
  size_t children_num;

  FieldDescriptor() = default;
  explicit FieldDescriptor(std::string name)
      : name(name), type(FieldType::kFeature), children_num(0) {}
  FieldDescriptor(std::string name, size_t children_num)
      : name(name), type(FieldType::kRawFeature), children_num(children_num) {}
};

absl::Status GenerateSchema(const std::vector<Row>& rows,
                            std::vector<FieldDescriptor>* fields);

absl::Status DumpWithParquetApi(arrow::MemoryPool* memory_pool,
                                CompressionMode compression_mode,
                                const std::vector<FieldDescriptor>& fields,
                                const std::vector<Row>& rows,
                                std::shared_ptr<arrow::io::OutputStream> sink);

absl::Status DumpWithParquetApiV2(
    arrow::MemoryPool* memory_pool, CompressionMode compression_mode,
    const std::vector<FieldDescriptor>& fields, const std::vector<Row>& rows,
    std::shared_ptr<arrow::io::OutputStream> sink);

absl::Status DumpWithArrowApi(arrow::MemoryPool* memory_pool,
                              CompressionMode compression_mode,
                              const std::vector<FieldDescriptor>& fields,
                              const std::vector<Row>& rows,
                              std::shared_ptr<arrow::io::OutputStream> sink);

}  // namespace feature_store
}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_DUMP_H_
