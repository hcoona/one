#ifndef CODELAB_PB_TO_PARQUET_CONVERTER_H_
#define CODELAB_PB_TO_PARQUET_CONVERTER_H_

#include <functional>
#include <string>

#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "parquet/api/schema.h"
#include "parquet/api/writer.h"

namespace hcoona {
namespace codelab {

absl::Status ConvertDescriptor(const google::protobuf::Descriptor* descriptor,
                               parquet::schema::NodeVector* fields);

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor* field_descriptor,
    parquet::schema::NodeVector* fields);

absl::Status WriteMessages(
    const google::protobuf::Descriptor* descriptor,
    absl::Span<const google::protobuf::Message> messages,
    parquet::RowGroupWriter* row_group_writer);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
