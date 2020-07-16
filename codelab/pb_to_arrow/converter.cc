#include "codelab/pb_to_arrow/converter.h"

#include <queue>
#include <utility>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "glog/logging.h"
#include "google/protobuf/message.h"
#include "codelab/pb_to_arrow/status_util.h"
#include "gtl/map_util.h"
#include "gtl/no_destructor.h"

namespace hcoona {
namespace codelab {

namespace {

// Provides a definition of base::queue that's like std::queue but uses a
// base::circular_deque instead of std::deque. Since std::queue is just a
// wrapper for an underlying type, we can just provide a typedef for it that
// defaults to the base circular_deque.
// template <class T, class Container = gtl::circular_deque<T>>
// using CircularQueue = std::queue<T, Container>;

// Implementation of C++20's std::identity.
//
// Reference:
// - https://en.cppreference.com/w/cpp/utility/functional/identity
// - https://wg21.link/func.identity
struct identity {
  template <typename T>
  constexpr T&& operator()(T&& t) const noexcept {
    return std::forward<T>(t);
  }

  using is_transparent = void;
};

const absl::flat_hash_map<google::protobuf::FieldDescriptor::Type,
                          std::function<std::shared_ptr<arrow::DataType>()>>*
GetArrowTypeTable() {
  static const gtl::NoDestructor<
      absl::flat_hash_map<google::protobuf::FieldDescriptor::Type,
                          std::function<std::shared_ptr<arrow::DataType>()>>>
      table({
          // Ordered by google::protobuf::FieldDescriptor::Type
          {
              google::protobuf::FieldDescriptor::Type::TYPE_DOUBLE,
              &arrow::float64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FLOAT,
              &arrow::float32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT64,
              &arrow::int64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT64,
              &arrow::uint64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_INT32,
              &arrow::int32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED64,
              &arrow::uint64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_FIXED32,
              &arrow::uint32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BOOL,
              &arrow::boolean,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_STRING,
              &arrow::utf8,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_BYTES,
              &arrow::binary,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_UINT32,
              &arrow::uint32,
          },
          {
              // Convert enum into string according to ProtoEnumConverter in
              // https://github.com/apache/parquet-mr/blob/2589cc821d2d470be1e79b86f511eb1f5fee4e5c/parquet-protobuf/src/main/java/org/apache/parquet/proto/ProtoMessageConverter.java#L187
              google::protobuf::FieldDescriptor::Type::TYPE_ENUM,
              &arrow::utf8,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED32,
              &arrow::int32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SFIXED64,
              &arrow::int64,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT32,
              &arrow::int32,
          },
          {
              google::protobuf::FieldDescriptor::Type::TYPE_SINT64,
              &arrow::int64,
          },
      });
  return table.get();
}

}  // namespace

absl::Status ConvertDescriptor(const google::protobuf::Descriptor* descriptor,
                               arrow::FieldVector* fields) {
  for (int i = 0; i < descriptor->field_count(); i++) {
    const google::protobuf::FieldDescriptor* field_descriptor =
        descriptor->field(i);
    std::shared_ptr<arrow::Field> field;
    absl::Status s = ConvertFieldDescriptor(field_descriptor, &field);
    if (!s.ok()) {
      return s;
    }

    fields->emplace_back(std::move(field));
  }

  return absl::OkStatus();
}

absl::Status ConvertFieldDescriptor(
    const google::protobuf::FieldDescriptor* field_descriptor,
    std::shared_ptr<arrow::Field>* field) {
  // TODO(zhangshuai.ustc): Distinguish map/list type for message/group.
  if (field_descriptor->type() ==
          google::protobuf::FieldDescriptor::TYPE_MESSAGE ||
      field_descriptor->type() ==
          google::protobuf::FieldDescriptor::TYPE_GROUP) {
    arrow::FieldVector fields;
    absl::Status s =
        ConvertDescriptor(field_descriptor->message_type(), &fields);
    if (!s.ok()) {
      return s;
    }

    std::shared_ptr<arrow::Field> arrow_field =
        arrow::field(field_descriptor->name(), arrow::struct_(fields));
    if (field_descriptor->is_repeated()) {
      *field = arrow::field(field_descriptor->name(),
                            arrow::list(std::move(arrow_field)));
    } else {
      *field = std::move(arrow_field);
    }
  } else {
    const std::function<std::shared_ptr<arrow::DataType>()>*
        arrow_type_factory =
            gtl::FindOrNull(*GetArrowTypeTable(), field_descriptor->type());
    if (arrow_type_factory == nullptr) {
      return absl::UnimplementedError(absl::StrCat(
          "Failed to convert '", field_descriptor->name(),
          "', not implemented for primitive type '",
          field_descriptor->type_name(), "(", field_descriptor->type(), ")'"));
    }

    if (field_descriptor->is_repeated()) {
      *field = arrow::field(field_descriptor->name(),
                            arrow::list(arrow_type_factory->operator()()),
                            true /* nullable */);
    } else {
      *field = arrow::field(field_descriptor->name(),
                            arrow_type_factory->operator()(),
                            field_descriptor->is_optional() /* nullable */);
    }
  }

  return absl::OkStatus();
}

absl::Status ConvertSchema(const google::protobuf::Descriptor* descriptor,
                           std::shared_ptr<arrow::Schema>* schema) {
  arrow::FieldVector fields;
  RETURN_STATUS_IF_NOT_OK(ConvertDescriptor(descriptor, &fields));
  *schema = arrow::schema(fields);
  return absl::OkStatus();
}

absl::Status ConvertFieldData(
    absl::Span<const google::protobuf::Message* const> messages,
    const google::protobuf::FieldDescriptor* field_descriptor,
    arrow::MemoryPool* pool,
    std::shared_ptr<arrow::Array>* messages_column_data_vector) {
  // TODO(zhangshuai.ustc): Distinguish map/list type for message/group.
  // if (field_descriptor->type() ==
  //         google::protobuf::FieldDescriptor::TYPE_MESSAGE ||
  //     field_descriptor->type() ==
  //         google::protobuf::FieldDescriptor::TYPE_GROUP) {

  //   arrow::FieldVector fields;
  //   absl::Status s =
  //       ConvertDescriptor(field_descriptor->message_type(), &fields);
  //   if (!s.ok()) {
  //     return s;
  //   }

  //   std::shared_ptr<arrow::Field> arrow_field =
  //       arrow::field(field_descriptor->name(), arrow::struct_(fields));
  //   if (field_descriptor->is_repeated()) {
  //     *field = arrow::field(field_descriptor->name(),
  //                           arrow::list(std::move(arrow_field)));
  //   } else {
  //     arrow::StructBuilder builder(pool);
  //   }
  // } else {
  //   const std::function<std::shared_ptr<arrow::DataType>()>*
  //       arrow_type_factory =
  //           gtl::FindOrNull(*GetArrowTypeTable(), field_descriptor->type());
  //   if (arrow_type_factory == nullptr) {
  //     return absl::UnimplementedError(absl::StrCat(
  //         "Failed to convert '", field_descriptor->name(),
  //         "', not implemented for primitive type '",
  //         field_descriptor->type_name(), "(", field_descriptor->type(), ")'"));
  //   }

  //   if (field_descriptor->is_repeated()) {
  //     *field = arrow::field(field_descriptor->name(),
  //                           arrow::list(arrow_type_factory->operator()()),
  //                           true /* nullable */);
  //   } else {
  //     *field = arrow::field(field_descriptor->name(),
  //                           arrow_type_factory->operator()(),
  //                           field_descriptor->is_optional() /* nullable */);
  //   }
  // }

  return absl::OkStatus();
}

absl::Status ConvertTable(
    const google::protobuf::Descriptor* descriptor,
    absl::Span<const google::protobuf::Message* const> messages,
    arrow::MemoryPool* pool, std::shared_ptr<arrow::Table>* table) {
  return absl::UnimplementedError("");
}

}  // namespace codelab
}  // namespace hcoona
