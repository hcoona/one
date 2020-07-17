#include "codelab/pb_to_arrow/converter.h"

#include <string>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "glog/logging.h"
#include "google/protobuf/message.h"
#include "codelab/pb_to_arrow/status_util.h"
#include "gtl/map_util.h"
#include "gtl/no_destructor.h"
#include "util/casts.h"

namespace hcoona {
namespace codelab {

namespace {

struct ArrowTypeAndBuilder {
  std::function<std::shared_ptr<arrow::DataType>()> type_factory;
  std::function<std::shared_ptr<arrow::ArrayBuilder>(arrow::MemoryPool*)>
      builder_factory;
};

template <typename T>
std::function<std::shared_ptr<arrow::ArrayBuilder>(arrow::MemoryPool*)>
MakeArrayBuilder() {
  return [](arrow::MemoryPool* pool) { return std::make_shared<T>(pool); };
}

const absl::flat_hash_map<google::protobuf::FieldDescriptor::Type,
                          ArrowTypeAndBuilder>*
GetArrowTypeTable() {
  static const gtl::NoDestructor<absl::flat_hash_map<
      google::protobuf::FieldDescriptor::Type, ArrowTypeAndBuilder>>
      table({
          // Ordered by google::protobuf::FieldDescriptor::Type
          {
              google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE,
              {
                  &arrow::float64,
                  MakeArrayBuilder<arrow::DoubleBuilder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FLOAT,
              {
                  &arrow::float32,
                  MakeArrayBuilder<arrow::FloatBuilder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT64,
              {
                  &arrow::int64,
                  MakeArrayBuilder<arrow::Int64Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT64,
              {
                  &arrow::uint64,
                  MakeArrayBuilder<arrow::UInt64Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT32,
              {
                  &arrow::int32,
                  MakeArrayBuilder<arrow::Int32Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED64,
              {
                  &arrow::uint64,
                  MakeArrayBuilder<arrow::UInt64Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED32,
              {
                  &arrow::uint32,
                  MakeArrayBuilder<arrow::UInt32Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BOOL,
              {
                  &arrow::boolean,
                  MakeArrayBuilder<arrow::BooleanBuilder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_STRING,
              {
                  &arrow::utf8,
                  MakeArrayBuilder<arrow::StringBuilder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BYTES,
              {
                  &arrow::binary,
                  MakeArrayBuilder<arrow::BinaryBuilder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT32,
              {
                  &arrow::uint32,
                  MakeArrayBuilder<arrow::UInt32Builder>(),
              },
          },
          {
              // Convert enum into string according to ProtoEnumConverter in
              // https://github.com/apache/parquet-mr/blob/2589cc821d2d470be1e79b86f511eb1f5fee4e5c/parquet-protobuf/src/main/java/org/apache/parquet/proto/ProtoMessageConverter.java#L187
              google::protobuf::FieldDescriptor::Type::TYPE_ENUM,
              {
                  &arrow::utf8,
                  MakeArrayBuilder<arrow::StringBuilder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32,
              {
                  &arrow::int32,
                  MakeArrayBuilder<arrow::Int32Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64,
              {
                  &arrow::int64,
                  MakeArrayBuilder<arrow::Int64Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT32,
              {
                  &arrow::int32,
                  MakeArrayBuilder<arrow::Int32Builder>(),
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT64,
              {
                  &arrow::int64,
                  MakeArrayBuilder<arrow::Int64Builder>(),
              },
          },
      });
  return table.get();
}

}  // namespace

absl::Status ConvertDescriptor(
    const google::protobuf::Descriptor& descriptor, arrow::MemoryPool* pool,
    std::vector<std::shared_ptr<arrow::Field>>* fields,
    std::vector<std::shared_ptr<arrow::ArrayBuilder>>* fields_builders) {
  DCHECK_NOTNULL(pool);
  DCHECK_NOTNULL(fields);
  DCHECK_NOTNULL(fields_builders);

  for (int i = 0; i < descriptor.field_count(); i++) {
    const google::protobuf::FieldDescriptor* field_descriptor =
        descriptor.field(i);
    std::shared_ptr<arrow::Field> field;
    std::shared_ptr<arrow::ArrayBuilder> field_builder;
    absl::Status s =
        ConvertFieldDescriptor(*field_descriptor, pool, &field, &field_builder);
    if (!s.ok()) {
      return s;
    }

    fields->emplace_back(std::move(field));
    fields_builders->emplace_back(std::move(field_builder));
  }

  return absl::OkStatus();
}

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor& field_descriptor,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Field>* field,
    std::shared_ptr<arrow::ArrayBuilder>* field_builder) {
  DCHECK_NOTNULL(pool);
  DCHECK_NOTNULL(field);
  DCHECK_NOTNULL(field_builder);

  if (field_descriptor.type() ==
          google::protobuf::FieldDescriptor::TYPE_MESSAGE ||
      field_descriptor.type() ==
          google::protobuf::FieldDescriptor::TYPE_GROUP) {
    std::vector<std::shared_ptr<arrow::Field>> fields;
    std::vector<std::shared_ptr<arrow::ArrayBuilder>> fields_builders;
    absl::Status s = ConvertDescriptor(*(field_descriptor.message_type()), pool,
                                       &fields, &fields_builders);
    if (!s.ok()) {
      return s;
    }

    if (field_descriptor.is_map()) {
      DCHECK_EQ(fields.size(), 2);
      DCHECK_EQ(fields_builders.size(), 2);

      *field = arrow::field(field_descriptor.name(),
                            arrow::map(fields[0]->type(), fields[1]->type()));
      *field_builder = std::make_shared<arrow::MapBuilder>(
          pool, fields_builders[0], fields_builders[1],
          arrow::map(fields[0]->type(), fields[1]->type()));
    } else {
      std::shared_ptr<arrow::DataType> arrow_struct_type =
          arrow::struct_(fields);
      auto struct_builder = std::make_shared<arrow::StructBuilder>(
          arrow_struct_type, pool, fields_builders);
      if (field_descriptor.is_repeated()) {
        *field = arrow::field(field_descriptor.name(),
                              arrow::list(arrow::struct_(fields)));
        *field_builder = std::make_shared<arrow::ListBuilder>(
            pool, std::move(struct_builder),
            arrow::list(std::move(arrow_struct_type)));
      } else {
        *field =
            arrow::field(field_descriptor.name(), std::move(arrow_struct_type));
        *field_builder = std::move(struct_builder);
      }
    }
  } else {
    const ArrowTypeAndBuilder* arrow_type_and_builder =
        gtl::FindOrNull(*GetArrowTypeTable(), field_descriptor.type());
    if (arrow_type_and_builder == nullptr) {
      return absl::UnimplementedError(absl::StrCat(
          "Failed to convert '", field_descriptor.name(),
          "', not implemented for primitive type '",
          field_descriptor.type_name(), "(", field_descriptor.type(), ")'"));
    }

    if (field_descriptor.is_map()) {
      return absl::UnimplementedError(absl::StrCat(
          __LINE__,
          "Not implemented for map: ", field_descriptor.DebugString()));
    } else if (field_descriptor.is_repeated()) {
      DCHECK_EQ(arrow_type_and_builder->builder_factory(pool)->type(),
                arrow_type_and_builder->type_factory());

      *field = arrow::field(field_descriptor.name(),
                            arrow::list(arrow_type_and_builder->type_factory()),
                            true /* nullable */);
      *field_builder = std::make_shared<arrow::ListBuilder>(
          pool, arrow_type_and_builder->builder_factory(pool),
          arrow::list(arrow_type_and_builder->type_factory()));
    } else {
      *field = arrow::field(field_descriptor.name(),
                            arrow_type_and_builder->type_factory(),
                            field_descriptor.is_optional() /* nullable */);
      *field_builder = arrow_type_and_builder->builder_factory(pool);
    }
  }

  return absl::OkStatus();
}

absl::Status ConvertData(
    const google::protobuf::Descriptor& descriptor,
    const google::protobuf::Message& message,
    const std::vector<std::shared_ptr<arrow::ArrayBuilder>>& fields_builders) {
  DCHECK_EQ(fields_builders.size(), descriptor.field_count());
#if DCHECK_IS_ON()
  for (const std::shared_ptr<arrow::ArrayBuilder>& builder : fields_builders) {
    CHECK(static_cast<bool>(builder));
  }
#endif  // DCHECK_IS_ON()

  for (int i = 0; i < descriptor.field_count(); i++) {
    const google::protobuf::FieldDescriptor* field_descriptor =
        descriptor.field(i);
    if (field_descriptor->is_map()) {
      auto field_builder =
          std::static_pointer_cast<arrow::MapBuilder>(fields_builders[i]);
      arrow::ArrayBuilder* key_builder = field_builder->key_builder();
      arrow::ArrayBuilder* value_builder = field_builder->value_builder();
      RETURN_STATUS_IF_NOT_OK(FromArrowStatus(field_builder->Append()));
      for (int j = 0;
           j < message.GetReflection()->FieldSize(message, field_descriptor);
           j++) {
        // Extract into key & value fields.
        const google::protobuf::Message& inner_message =
            message.GetReflection()->GetRepeatedMessage(message,
                                                        field_descriptor, j);
        LOG(INFO) << "Inner message: " << inner_message.DebugString();
        return absl::UnimplementedError(absl::StrCat(
            __LINE__,
            "Not implemented for map: ", field_descriptor->DebugString()));
        // RETURN_STATUS_IF_NOT_OK(
        //     ConvertFieldData(*field_descriptor, message, j, value_builder));
      }
      // return absl::UnimplementedError(absl::StrCat(
      //     __LINE__,
      //     "Not implemented for map: ", field_descriptor->DebugString()));
    } else if (field_descriptor->is_repeated()) {
      auto field_builder =
          std::static_pointer_cast<arrow::ListBuilder>(fields_builders[i]);
      arrow::ArrayBuilder* value_builder = field_builder->value_builder();
      RETURN_STATUS_IF_NOT_OK(FromArrowStatus(field_builder->Append()));
      for (int j = 0;
           j < message.GetReflection()->FieldSize(message, field_descriptor);
           j++) {
        RETURN_STATUS_IF_NOT_OK(
            ConvertFieldData(*field_descriptor, message, j, value_builder));
      }
    } else {
      const std::shared_ptr<arrow::ArrayBuilder>& field_builder =
          fields_builders[i];
      if (field_descriptor->is_optional() &&
          !message.GetReflection()->HasField(message, field_descriptor)) {
        RETURN_STATUS_IF_NOT_OK(FromArrowStatus(field_builder->AppendNull()));
      } else {
        RETURN_STATUS_IF_NOT_OK(ConvertFieldData(*field_descriptor, message, -1,
                                                 field_builder.get()));
      }
    }
  }

  return absl::OkStatus();
}

absl::Status ConvertFieldData(
    const google::protobuf::FieldDescriptor& field_descriptor,
    const google::protobuf::Message& message, int repeated_field_index,
    arrow::ArrayBuilder* field_builder) {
  DCHECK_NOTNULL(field_builder);
#if DCHECK_IS_ON()
  if (field_descriptor.is_repeated()) {
    CHECK_LT(repeated_field_index,
             message.GetReflection()->FieldSize(message, &field_descriptor));
  } else {
    CHECK_EQ(repeated_field_index, -1);
  }
#endif  // DCHECK_IS_ON()

  switch (field_descriptor.type()) {
    case google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE: {
      auto builder = down_cast<arrow::DoubleBuilder*>(field_builder);
      double value =
          repeated_field_index == -1
              ? message.GetReflection()->GetDouble(message, &field_descriptor)
              : message.GetReflection()->GetRepeatedDouble(
                    message, &field_descriptor, repeated_field_index);
      RETURN_STATUS_IF_NOT_OK(
          FromArrowStatus(builder->Append(std::move(value))));
      break;
    }
    case google::protobuf::FieldDescriptor::Type::TYPE_FLOAT:
      return absl::UnimplementedError(absl::StrCat(
          "Not implemented for other types converting field data: ",
          field_descriptor.type_name()));
    case google::protobuf::FieldDescriptor::Type::TYPE_INT64: {
      auto builder = down_cast<arrow::Int64Builder*>(field_builder);
      int64_t value =
          repeated_field_index == -1
              ? message.GetReflection()->GetInt64(message, &field_descriptor)
              : message.GetReflection()->GetRepeatedInt64(
                    message, &field_descriptor, repeated_field_index);
      RETURN_STATUS_IF_NOT_OK(
          FromArrowStatus(builder->Append(std::move(value))));
      break;
    }
    case google::protobuf::FieldDescriptor::Type::TYPE_UINT64:
      return absl::UnimplementedError(absl::StrCat(
          "Not implemented for other types converting field data: ",
          field_descriptor.type_name()));
    case google::protobuf::FieldDescriptor::Type::TYPE_INT32: {
      auto builder = down_cast<arrow::Int32Builder*>(field_builder);
      int32_t value =
          repeated_field_index == -1
              ? message.GetReflection()->GetInt32(message, &field_descriptor)
              : message.GetReflection()->GetRepeatedInt32(
                    message, &field_descriptor, repeated_field_index);
      RETURN_STATUS_IF_NOT_OK(
          FromArrowStatus(builder->Append(std::move(value))));
      break;
    }
    case google::protobuf::FieldDescriptor::Type::TYPE_ENUM: {
      auto builder = down_cast<arrow::StringBuilder*>(field_builder);
      std::string value =
          repeated_field_index == -1
              ? message.GetReflection()
                    ->GetEnum(message, &field_descriptor)
                    ->name()
              : message.GetReflection()
                    ->GetRepeatedEnum(message, &field_descriptor,
                                      repeated_field_index)
                    ->name();
      RETURN_STATUS_IF_NOT_OK(
          FromArrowStatus(builder->Append(std::move(value))));
      break;
    }
    default:
      return absl::UnimplementedError(absl::StrCat(
          "Not implemented for other types converting field data: ",
          field_descriptor.type_name()));
  }

  return absl::OkStatus();
}

absl::Status ConvertTable(
    const google::protobuf::Descriptor& descriptor,
    absl::Span<const google::protobuf::Message* const> messages,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Table>* table) {
  DCHECK_NOTNULL(pool);
  DCHECK_NOTNULL(table);

  std::vector<std::shared_ptr<arrow::Field>> fields;
  std::vector<std::shared_ptr<arrow::ArrayBuilder>> fields_builders;
  RETURN_STATUS_IF_NOT_OK(
      ConvertDescriptor(descriptor, pool, &fields, &fields_builders));
  std::shared_ptr<arrow::Schema> schema = arrow::schema(fields);

  for (const google::protobuf::Message* const message : messages) {
    RETURN_STATUS_IF_NOT_OK(ConvertData(descriptor, *message, fields_builders));
  }

  std::vector<std::shared_ptr<arrow::Array>> arrays;
  arrays.reserve(fields_builders.size());
  for (std::size_t i = 0; i < fields_builders.size(); i++) {
    std::shared_ptr<arrow::Array> array;
    RETURN_STATUS_IF_NOT_OK(
        FromArrowStatus(fields_builders[i]->Finish(&array)));
    arrays.emplace_back(std::move(array));
  }
  *table = arrow::Table::Make(schema, arrays, messages.size());

  return absl::OkStatus();
}

}  // namespace codelab
}  // namespace hcoona
