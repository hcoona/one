#ifndef CODELAB_PB_TO_PARQUET_CONVERTER_H_
#define CODELAB_PB_TO_PARQUET_CONVERTER_H_

#include <functional>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/types/span.h"
#include "arrow/schema.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "parquet/arrow/writer.h"

namespace hcoona {
namespace codelab {

absl::Status ConvertDescriptor(const google::protobuf::Descriptor* descriptor,
                               arrow::FieldVector* fields);

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor* field_descriptor,
    std::shared_ptr<arrow::Field>* field);

absl::Status ConvertSchema(const google::protobuf::Descriptor* descriptor,
                           std::shared_ptr<arrow::Schema>* schema);

absl::Status ConvertFieldData(
    absl::Span<const google::protobuf::Message* const> messages,
    const google::protobuf::FieldDescriptor* field_descriptor,
    arrow::MemoryPool* pool,
    std::shared_ptr<arrow::Array>* messages_column_data_vector);

absl::Status ConvertTable(
    const google::protobuf::Descriptor* descriptor,
    absl::Span<const google::protobuf::Message* const> messages,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Table>* table);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
