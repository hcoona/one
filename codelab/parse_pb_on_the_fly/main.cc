#include <string>

#include "absl/status/status.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/dynamic_message.h"
#include "gtl/file_system.h"
#include "gtl/macros.h"
#include "gtl/posix_file_system.h"

namespace {

bool FlagStringNotEmpty(const char* flag_name, const std::string& value) {
  return !value.empty();
}

}  // namespace

DEFINE_string(proto_file, "", "Protobuf description file.");
DEFINE_validator(proto_file, &FlagStringNotEmpty);

DEFINE_string(message_name, "",
              "Message name in the protobuf description file.");
DEFINE_validator(message_name, &FlagStringNotEmpty);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  gtl::PosixFileSystem file_system;
  absl::Status s = file_system.FileExists(FLAGS_proto_file);
  if (!s.ok()) {
    LOG(ERROR) << "Specified file '" << FLAGS_proto_file
               << "' not exists: " << s.ToString();
    return 1;
  }

  google::protobuf::compiler::DiskSourceTree source_tree;
  source_tree.MapPath("", std::string(file_system.Dirname(FLAGS_proto_file)));
  google::protobuf::compiler::Importer importer(&source_tree,
                                                nullptr /* error_collector */);
  const google::protobuf::FileDescriptor* file_descriptor =
      importer.Import(std::string(file_system.Basename(FLAGS_proto_file)));
  if (file_descriptor == nullptr) {
    LOG(ERROR) << "Failed to parse file '" << FLAGS_proto_file << "'";
    return 2;
  }

  const google::protobuf::Descriptor* descriptor =
      importer.pool()->FindMessageTypeByName(FLAGS_message_name);
  if (descriptor == nullptr) {
    LOG(ERROR) << "Failed to find '" << FLAGS_message_name << "' in '"
               << FLAGS_proto_file << "'";
    return 3;
  }

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
      DCHECK(enum_descriptor);
    }
  }

  google::protobuf::DynamicMessageFactory dynamic_message_factory;
  const google::protobuf::Message* message =
      dynamic_message_factory.GetPrototype(descriptor);
  ignore_result(message);

  return 0;
}
