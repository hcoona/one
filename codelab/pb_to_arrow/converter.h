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

absl::Status WriteMessages(const google::protobuf::Descriptor* descriptor,
                           absl::Span<const google::protobuf::Message> messages,
                           parquet::arrow::FileWriter* file_writer);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
