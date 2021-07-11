#ifndef CODELAB_PB_TO_PARQUET_CONVERTER_H_
#define CODELAB_PB_TO_PARQUET_CONVERTER_H_

#include <functional>
#include <string>

#include "third_party/absl/status/status.h"
#include "third_party/absl/types/optional.h"
#include "third_party/absl/types/span.h"
#include "third_party/arrow/src/parquet/api/schema.h"
#include "third_party/arrow/src/parquet/api/writer.h"
#include "third_party/protobuf/src/google/protobuf/descriptor.h"
#include "third_party/protobuf/src/google/protobuf/message.h"

namespace hcoona {
namespace codelab {

absl::Status ConvertDescriptor(const google::protobuf::Descriptor* descriptor,
                               parquet::schema::NodeVector* fields);

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor* field_descriptor,
    parquet::schema::NodeVector* fields);

absl::Status WriteMessages(
    const google::protobuf::Descriptor* descriptor,
    absl::Span<const google::protobuf::Message*> messages,
    parquet::RowGroupWriter* row_group_writer);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
