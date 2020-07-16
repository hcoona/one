#ifndef CODELAB_PB_TO_PARQUET_CONVERTER_H_
#define CODELAB_PB_TO_PARQUET_CONVERTER_H_

#include <memory>
#include <vector>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "arrow/api.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "parquet/arrow/writer.h"

namespace hcoona {
namespace codelab {

absl::Status ConvertDescriptor(
    const google::protobuf::Descriptor& descriptor, arrow::MemoryPool* pool,
    std::vector<std::shared_ptr<arrow::Field>>* fields,
    std::vector<std::shared_ptr<arrow::ArrayBuilder>>* fields_builders);

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor& field_descriptor,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Field>* field,
    std::shared_ptr<arrow::ArrayBuilder>* field_builder);

absl::Status ConvertData(
    const google::protobuf::Descriptor& descriptor,
    const google::protobuf::Message& message,
    const std::vector<std::shared_ptr<arrow::ArrayBuilder>>& fields_builders);

absl::Status ConvertFieldData(
    const google::protobuf::FieldDescriptor& field_descriptor,
    const google::protobuf::Message& message, int repeated_field_index,
    arrow::ArrayBuilder* field_builder);

absl::Status ConvertTable(
    const google::protobuf::Descriptor& descriptor,
    absl::Span<const google::protobuf::Message* const> messages,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Table>* table);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
