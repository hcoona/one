#include "one/codelab/pb_to_parquet/converter.h"

#include <utility>

#include "third_party/glog/logging.h"
#include "third_party/absl/container/flat_hash_map.h"
#include "third_party/absl/strings/str_cat.h"
#include "gtl/map_util.h"
#include "gtl/no_destructor.h"

namespace hcoona {
namespace codelab {

namespace {

const absl::flat_hash_map<
    google::protobuf::FieldDescriptor::Type,
    std::function<parquet::schema::NodePtr(
        const std::string& /* field_name */,
        parquet::Repetition::type /* required/optional/repeated */,
        int /* field_id */)>>*
GetFieldMakerTable() {
  static const gtl::NoDestructor<absl::flat_hash_map<
      google::protobuf::FieldDescriptor::Type,
      std::function<parquet::schema::NodePtr(
          const std::string& /* field name */,
          parquet::Repetition::type /* required/optional/repeated */,
          int /* field id */)>>>
      table({
          // Ordered by google::protobuf::FieldDescriptor::Type
          {
              google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::DOUBLE,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FLOAT,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::FLOAT,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT64,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::INT64,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT64,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition,
                    parquet::LogicalType::Int(64, false), parquet::Type::INT64,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT32,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::INT32,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED64,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition,
                    parquet::LogicalType::Int(64, false), parquet::Type::INT64,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED32,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition,
                    parquet::LogicalType::Int(32, false /* is_signed */),
                    parquet::Type::INT32,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BOOL,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::BOOLEAN,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_STRING,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::String(),
                    parquet::Type::BYTE_ARRAY,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BYTES,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::BYTE_ARRAY,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT32,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition,
                    parquet::LogicalType::Int(32, false), parquet::Type::INT32,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_ENUM,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::Enum(),
                    parquet::Type::BYTE_ARRAY,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::INT32,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::INT64,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT32,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::INT32,
                    /*length=*/-1, field_id);
              },
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT64,
              [](const std::string& field_name,
                 parquet::Repetition::type repetition, int field_id) {
                return parquet::schema::PrimitiveNode::Make(
                    field_name, repetition, parquet::LogicalType::None(),
                    parquet::Type::INT64,
                    /*length=*/-1, field_id);
              },
          },
      });
  return table.get();
}

const absl::flat_hash_map<
    google::protobuf::FieldDescriptor::Type /* protobuf_type */,
    parquet::Type::type /* parquet_type */>*
GetPrimitiveTypeTable() {
  static const gtl::NoDestructor<absl::flat_hash_map<
      google::protobuf::FieldDescriptor::Type /* protobuf_type */,
      parquet::Type::type /* parquet_type */>>
      table({
          // Ordered by google::protobuf::FieldDescriptor::Type
          {
              google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE,
              parquet::Type::DOUBLE,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FLOAT,
              parquet::Type::FLOAT,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT64,
              parquet::Type::INT64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT64,
              parquet::Type::INT64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT32,
              parquet::Type::INT32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED64,
              parquet::Type::INT64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED32,
              parquet::Type::INT32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BOOL,
              parquet::Type::BOOLEAN,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_STRING,
              parquet::Type::BYTE_ARRAY,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BYTES,
              parquet::Type::BYTE_ARRAY,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT32,
              parquet::Type::INT32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_ENUM,
              parquet::Type::BYTE_ARRAY,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32,
              parquet::Type::INT32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64,
              parquet::Type::INT64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT32,
              parquet::Type::INT32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT64,
              parquet::Type::INT64,
          },
      });
  return table.get();
}

parquet::Repetition::type ComputeRepetitionType(
    const google::protobuf::FieldDescriptor* field_descriptor) {
  if (field_descriptor->is_required()) {
    return parquet::Repetition::type::REQUIRED;
  } else if (field_descriptor->is_optional()) {
    return parquet::Repetition::type::OPTIONAL;
  } else if (field_descriptor->is_repeated()) {
    return parquet::Repetition::type::REPEATED;
  } else {
    return parquet::Repetition::type::UNDEFINED;
  }
}

}  // namespace

absl::Status ConvertDescriptor(const google::protobuf::Descriptor* descriptor,
                               parquet::schema::NodeVector* fields) {
  for (int i = 0; i < descriptor->field_count(); i++) {
    const google::protobuf::FieldDescriptor* field_descriptor =
        descriptor->field(i);
    absl::Status s =
        hcoona::codelab::ConvertFieldDescriptor(field_descriptor, fields);
    if (!s.ok()) {
      return s;
    }
  }

  return absl::OkStatus();
}

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor* field_descriptor,
    parquet::schema::NodeVector* fields) {
  if (field_descriptor->type() ==
          google::protobuf::FieldDescriptor::TYPE_MESSAGE ||
      field_descriptor->type() ==
          google::protobuf::FieldDescriptor::TYPE_GROUP) {
    parquet::schema::NodeVector nested_fields;
    absl::Status s =
        ConvertDescriptor(field_descriptor->message_type(), &nested_fields);
    if (!s.ok()) {
      s;
    }

    fields->emplace_back(parquet::schema::GroupNode::Make(
        field_descriptor->name(), ComputeRepetitionType(field_descriptor),
        nested_fields, nullptr, field_descriptor->number()));
  } else {
    const std::function<parquet::schema::NodePtr(
        const std::string& /* field_name */,
        parquet::Repetition::type /* required/optional/repeated */,
        int /* field_id */)>* f =
        gtl::FindOrNull(*GetFieldMakerTable(), field_descriptor->type());
    if (f == nullptr) {
      return absl::UnimplementedError(absl::StrCat(
          "Failed to convert '", field_descriptor->name(),
          "', not implemented for primitive type '",
          field_descriptor->type_name(), "(", field_descriptor->type(), ")'"));
    }

    parquet::schema::NodePtr node = f->operator()(
        field_descriptor->name(), ComputeRepetitionType(field_descriptor),
        field_descriptor->number());
    fields->emplace_back(std::move(node));
  }
  return absl::OkStatus();
}

absl::Status WriteMessages(
    const google::protobuf::Descriptor* descriptor,
    absl::Span<const google::protobuf::Message*> messages,
    parquet::RowGroupWriter* row_group_writer) {
  for (int i = 0; i < descriptor->field_count(); i++) {
    const google::protobuf::FieldDescriptor* field_descriptor =
        descriptor->field(i);

    const parquet::Type::type* parquet_type =
        gtl::FindOrNull(*GetPrimitiveTypeTable(), field_descriptor->type());
    if (parquet_type == nullptr) {
      return absl::UnimplementedError(absl::StrCat(
          "Failed to convert '", field_descriptor->name(),
          "', not implemented for primitive type '",
          field_descriptor->type_name(), "(", field_descriptor->type(), ")'"));
    }

    parquet::ColumnWriter* column_writer = row_group_writer->column(i);
    const google::protobuf::Reflection* r = messages[0]->GetReflection();
  }

  return absl::OkStatus();
}

}  // namespace codelab
}  // namespace hcoona
