#include <string>

#include "gflags/gflags.h"
#include "third_party/glog/logging.h"
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/message.h"
#include "google/protobuf/reflection.h"
#include "google/protobuf/repeated_field.h"
#include "third_party/absl/container/flat_hash_map.h"
#include "third_party/absl/status/status.h"
#include "third_party/arrow/src/parquet/api/io.h"
#include "third_party/arrow/src/parquet/api/schema.h"
#include "third_party/arrow/src/parquet/api/writer.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/map_util.h"
#include "gtl/posix_file_system.h"
#include "one/codelab/pb_to_parquet/converter.h"

namespace {

constexpr const char kInputFile[] =
    "com_github_hcoona_one/codelab/pb_to_parquet/messages.proto";

bool FlagStringNotEmpty(const char* flag_name, const std::string& value) {
  ignore_result(flag_name);
  return !value.empty();
}

}  // namespace

DEFINE_string(proto_file, "", "Protobuf description file.");
DEFINE_string(message_name, "codelab.MessageA",
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

  parquet::schema::NodeVector fields;
  s = hcoona::codelab::ConvertDescriptor(descriptor, &fields);
  if (!s.ok()) {
    LOG(FATAL) << "Failed to convert protobuf descriptor, descriptor="
               << descriptor->DebugString() << ", message=" << s.ToString();
  }
  parquet::schema::NodePtr root = parquet::schema::GroupNode::Make(
      "schema", parquet::Repetition::REPEATED, fields);
  parquet::SchemaDescriptor schema;
  schema.Init(root);

  LOG(INFO) << "Protobuf Schema: " << descriptor->DebugString();
  LOG(INFO) << "Parquet Schema: " << schema.ToString();

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
  LOG(INFO) << message->Utf8DebugString();

  return 0;
}
