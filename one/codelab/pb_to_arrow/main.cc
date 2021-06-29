#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "google/protobuf/reflection.h"
#include "google/protobuf/repeated_field.h"
#include "third_party/absl/container/flat_hash_map.h"
#include "third_party/absl/status/status.h"
#include "third_party/absl/types/span.h"
#include "third_party/arrow/src/arrow/api.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/map_util.h"
#include "gtl/posix_file_system.h"
#include "one/base/macros.h"
#include "one/codelab/pb_to_arrow/converter.h"
#include "one/codelab/pb_to_arrow/status_util.h"

namespace {

constexpr const char kInputFile[] =
    "com_github_hcoona_one/codelab/pb_to_arrow/messages.proto";

bool FlagStringNotEmpty(const char* flag_name, const std::string& value) {
  ignore_result(flag_name);
  return !value.empty();
}

}  // namespace

DEFINE_string(proto_file, "", "Protobuf description file.");
DEFINE_string(message_name, "hcoona.codelab.MessageA",
              "Message name in the protobuf description file.");
DEFINE_validator(message_name, &FlagStringNotEmpty);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  std::string error_message;
  std::unique_ptr<bazel::tools::cpp::runfiles::Runfiles> runfiles(
      bazel::tools::cpp::runfiles::Runfiles::Create(argv[0], &error_message));
  CHECK(runfiles) << "Failed to create Bazel runfiles context: "
                  << error_message;

  std::string proto_file = FLAGS_proto_file.empty()
                               ? runfiles->Rlocation(kInputFile)
                               : FLAGS_proto_file;
  std::string message_name = FLAGS_message_name;

  gtl::PosixFileSystem file_system;
  absl::Status s = file_system.FileExists(proto_file);
  CHECK(s.ok()) << "Specified file '" << proto_file
                << "' not exists: " << s.ToString();

  google::protobuf::compiler::DiskSourceTree source_tree;
  source_tree.MapPath("", std::string(file_system.Dirname(proto_file)));
  google::protobuf::compiler::Importer importer(&source_tree,
                                                nullptr /* error_collector */);
  const google::protobuf::FileDescriptor* file_descriptor =
      importer.Import(std::string(file_system.Basename(proto_file)));
  CHECK(file_descriptor) << "Failed to parse file '" << proto_file << "'";

  const google::protobuf::Descriptor* descriptor =
      importer.pool()->FindMessageTypeByName(message_name);
  CHECK(descriptor) << "Failed to find '" << message_name << "' in '"
                    << proto_file << "'";

  arrow::MemoryPool* pool = arrow::default_memory_pool();

  std::vector<std::shared_ptr<arrow::Field>> fields;
  std::vector<std::shared_ptr<arrow::ArrayBuilder>> fields_builders;
  s = hcoona::codelab::ConvertDescriptor(*descriptor, pool, &fields,
                                         &fields_builders);
  CHECK(s.ok()) << "Failed to convert protobuf descriptor, descriptor="
                << descriptor->DebugString() << ", message=" << s.ToString();
  std::shared_ptr<arrow::Schema> schema = arrow::schema(fields);

  LOG(INFO) << "Protobuf Schema: " << descriptor->DebugString();
  LOG(INFO) << "Arrow Schema: " << schema->ToString();

  google::protobuf::DynamicMessageFactory dynamic_message_factory;
  const google::protobuf::Message* message_prototype =
      dynamic_message_factory.GetPrototype(descriptor);
  DCHECK_NOTNULL(message_prototype);

  google::protobuf::Message* message = message_prototype->New();
  message_prototype->GetReflection()->SetInt32(
      message, descriptor->FindFieldByNumber(1), 2);
  message_prototype->GetReflection()->SetInt64(
      message, descriptor->FindFieldByNumber(2), 3);
  google::protobuf::MutableRepeatedFieldRef<int32_t> ids =
      message_prototype->GetReflection()->GetMutableRepeatedFieldRef<int32_t>(
          message, descriptor->FindFieldByNumber(100));
  ids.Add(2);
  ids.Add(3);
  ids.Add(5);
  ids.Add(7);
  message_prototype->GetReflection()->SetEnumValue(
      message, descriptor->FindFieldByNumber(8), 2);
  // google::protobuf::Message* message_c =
  //     message_prototype->GetReflection()->MutableMessage(
  //         message, descriptor->FindFieldByNumber(103));
  // google::protobuf::MutableRepeatedFieldRef<int64_t> my_sint64_value =
  //     message_c->GetReflection()->GetMutableRepeatedFieldRef<int64_t>(
  //         message_c, message_c->GetDescriptor()->FindFieldByNumber(1));
  // my_sint64_value.Add(23);
  // my_sint64_value.Add(29);
  LOG(INFO) << "Constructed protobuf message: " << message->Utf8DebugString();

  arrow::StructBuilder message_c_builder(
      arrow::struct_(
          {arrow::field("my_sint64_value", arrow::list(arrow::int64()))}),
      pool,
      {std::make_shared<arrow::ListBuilder>(
          pool, std::make_shared<arrow::Int64Builder>(pool))});
  arrow::ListBuilder* inner_builder =
      static_cast<arrow::ListBuilder*>(message_c_builder.field_builder(0));
  arrow::Int64Builder* inner_inner_builder =
      static_cast<arrow::Int64Builder*>(inner_builder->value_builder());
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(message_c_builder.Append()));
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(inner_builder->Append()));
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(inner_inner_builder->Append(23)));
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(inner_inner_builder->Append(29)));
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::FromArrowStatus(inner_inner_builder->Append(31)));

  std::shared_ptr<arrow::StructArray> message_c_column_trunk;
  ONE_CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(
      message_c_builder.Finish(&message_c_column_trunk)));
  LOG(INFO) << "message_c_column_trunk: " << message_c_column_trunk->ToString();

  // const google::protobuf::EnumValueDescriptor* enum_value_descriptor =
  //     message_prototype->GetReflection()->GetEnum(
  //         *message, descriptor->FindFieldByNumber(8));
  // LOG(INFO) << enum_value_descriptor->name();

  absl::Span<const google::protobuf::Message* const> messages =
      absl::MakeConstSpan(&message, 1);

  std::shared_ptr<arrow::Table> table;
  ONE_CHECK_STATUS_OK(
      hcoona::codelab::ConvertTable(*descriptor, messages, pool, &table));
  LOG(INFO) << "Table: " << table->ToString();
  ONE_CHECK_STATUS_OK(hcoona::codelab::FromArrowStatus(table->ValidateFull()));

  return 0;
}
