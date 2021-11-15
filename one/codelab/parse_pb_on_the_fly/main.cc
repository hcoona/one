#include <string>

#include "absl/status/status.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/dynamic_message.h"
#include "tools/cpp/runfiles/runfiles.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/posix_file_system.h"

namespace {

constexpr const char kInputFile[] =
    "com_github_hcoona_one/one/codelab/parse_pb_on_the_fly/messages.proto";

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

  for (int i = 0; i < descriptor->field_count(); i++) {
    const google::protobuf::FieldDescriptor* field_descriptor =
        descriptor->field(i);
    LOG(INFO) << "Field(" << i << "): type=" << field_descriptor->type()
              << ", optional=" << field_descriptor->is_optional()
              << ", repeated=" << field_descriptor->is_repeated();
    if (field_descriptor->type() ==
        google::protobuf::FieldDescriptor::TYPE_ENUM) {
      const google::protobuf::EnumDescriptor* enum_descriptor =
          field_descriptor->enum_type();
      DCHECK_NOTNULL(enum_descriptor);
    }
  }

  google::protobuf::DynamicMessageFactory dynamic_message_factory;
  const google::protobuf::Message* message =
      dynamic_message_factory.GetPrototype(descriptor);
  DCHECK_NOTNULL(message);

  return 0;
}
