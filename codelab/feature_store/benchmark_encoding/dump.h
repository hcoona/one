#ifndef CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_DUMP_H_
#define CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_DUMP_H_

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "codelab/feature_store/benchmark_encoding/row.h"

namespace codelab {
namespace feature_store {

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

absl::Status DumpWithParquetApi(const std::vector<FieldDescriptor>& fields,
                                const std::vector<Row>& rows,
                                int64_t* written_bytes);

absl::Status DumpWithParquetApiV2(const std::vector<FieldDescriptor>& fields,
                                  const std::vector<Row>& rows,
                                  int64_t* written_bytes);

absl::Status DumpWithArrowApi(const std::vector<Row>& rows);

}  // namespace feature_store
}  // namespace codelab

#endif  // CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_DUMP_H_
