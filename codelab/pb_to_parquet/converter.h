#ifndef CODELAB_PB_TO_PARQUET_CONVERTER_H_
#define CODELAB_PB_TO_PARQUET_CONVERTER_H_

#include <functional>
#include <string>

#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "google/protobuf/descriptor.h"
#include "parquet/api/schema.h"

namespace hcoona {
namespace codelab {

absl::Status ConvertDescriptor(const google::protobuf::Descriptor* descriptor,
                               parquet::schema::NodeVector* fields);

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor* field_descriptor,
    parquet::schema::NodeVector* fields);

}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_PB_TO_PARQUET_CONVERTER_H_
