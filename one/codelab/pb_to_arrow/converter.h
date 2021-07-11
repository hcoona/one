#ifndef CODELAB_PB_TO_PARQUET_CONVERTER_H_
#define CODELAB_PB_TO_PARQUET_CONVERTER_H_

#include <memory>
#include <string>
#include <vector>

#include "third_party/absl/status/status.h"
#include "third_party/absl/types/span.h"
#include "third_party/arrow/src/arrow/api.h"
#include "third_party/arrow/src/parquet/arrow/writer.h"
#include "third_party/protobuf/src/google/protobuf/descriptor.h"
#include "third_party/protobuf/src/google/protobuf/message.h"

namespace hcoona {
namespace codelab {

std::string LookupArrowArrayBuilderDebugLocation(arrow::ArrayBuilder* builder);

void ClearArrowArrayBuilderDebugContext();

absl::Status ConvertDescriptor(
    const google::protobuf::Descriptor& descriptor, arrow::MemoryPool* pool,
    std::vector<std::shared_ptr<arrow::Field>>* fields,
    std::vector<std::shared_ptr<arrow::ArrayBuilder>>* fields_builders);

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor& field_descriptor,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Field>* field,
    std::shared_ptr<arrow::ArrayBuilder>* field_builder);

absl::Status ConvertPrimitiveFieldDescriptor(
    const google::protobuf::FieldDescriptor& field_descriptor,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Field>* field,
    std::shared_ptr<arrow::ArrayBuilder>* field_builder);

absl::Status ConvertData(
    const google::protobuf::Descriptor& descriptor,
    const google::protobuf::Message& message,
    absl::Span<arrow::ArrayBuilder* const> fields_builders);

absl::Status ConvertNullData(
    const google::protobuf::Descriptor& descriptor,
    absl::Span<arrow::ArrayBuilder* const> fields_builders);

absl::Status ConvertFieldData(
    const google::protobuf::FieldDescriptor& field_descriptor,
    const google::protobuf::Message& message, int repeated_field_index,
    arrow::ArrayBuilder* field_builder);

absl::Status ConvertNullFieldData(
    const google::protobuf::FieldDescriptor& field_descriptor,
    arrow::ArrayBuilder* field_builder);

absl::Status ConvertTable(
    const google::protobuf::Descriptor& descriptor,
    absl::Span<const google::protobuf::Message* const> messages,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Table>* table);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
