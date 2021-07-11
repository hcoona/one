#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "third_party/absl/status/status.h"
#include "third_party/arrow/src/arrow/api.h"
#include "third_party/arrow/src/arrow/io/api.h"
#include "third_party/arrow/src/arrow/io/file.h"
#include "third_party/arrow/src/parquet/api/io.h"
#include "third_party/arrow/src/parquet/api/schema.h"
#include "third_party/arrow/src/parquet/api/writer.h"
#include "third_party/arrow/src/parquet/arrow/writer.h"
#include "third_party/gflags/gflags.h"
#include "third_party/glog/logging.h"
#include "gtl/macros.h"

#define ARROW_CHECK_OK(status)                                        \
  do {                                                                \
    ::arrow::Status __s = ::arrow::internal::GenericToStatus(status); \
    CHECK(__s.ok()) << __s.ToString();                                \
  } while (false)

namespace {

bool FlagStringNotEmpty(const char* flag_name, const std::string& value) {
  ignore_result(flag_name);
  return !value.empty();
}

}  // namespace

DEFINE_string(output, "parquet_test_2.parquet", "Output file.");
DEFINE_validator(output, &FlagStringNotEmpty);

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  // Parquet-CLI cannot read repeated field directly. This is a by-design
  // behaviour. See further details in
  // https://github.com/apache/parquet-mr/blob/apache-parquet-1.11.0/parquet-protobuf/src/main/java/org/apache/parquet/proto/ProtoMessageConverter.java#L382

  // See further details about definition level & repetition level in
  // https://blog.twitter.com/engineering/en_us/a/2013/dremel-made-simple-with-parquet.html

  parquet::schema::NodeVector fields;
  fields.emplace_back(parquet::schema::PrimitiveNode::Make(
      "my_int32_array", parquet::Repetition::REPEATED,
      parquet::LogicalType::None(), parquet::Type::INT32, -1, 1));

  std::shared_ptr<arrow::io::FileOutputStream> output_file =
      arrow::io::FileOutputStream::Open(FLAGS_output).ValueOrDie();
  std::unique_ptr<parquet::ParquetFileWriter> file_writer =
      parquet::ParquetFileWriter::Open(
          output_file,
          std::static_pointer_cast<parquet::schema::GroupNode>(
              parquet::schema::GroupNode::Make(
                  "__schema", parquet::Repetition::REQUIRED, fields)));
  CHECK(file_writer);

  LOG(INFO) << "Schema: " << file_writer->schema()->ToString();

  parquet::RowGroupWriter* row_group_writer = file_writer->AppendRowGroup(2);
  CHECK(row_group_writer);

  parquet::Int32Writer* column_writer =
      static_cast<parquet::Int32Writer*>(row_group_writer->NextColumn());
  CHECK(column_writer);

  LOG(INFO) << column_writer->descr()->ToString();
  std::array<int32_t, 6> values = {17, 19, 23, 27, 31, 37};
  int16_t definition_level = 1;
  int16_t repetition_level = 0;
  column_writer->WriteBatch(1, &definition_level, &repetition_level,
                            &values[0]);

  definition_level = 1;
  repetition_level = 1;
  column_writer->WriteBatch(1, &definition_level, &repetition_level,
                            &values[1]);
  column_writer->WriteBatch(1, &definition_level, &repetition_level,
                            &values[2]);

  definition_level = 1;
  repetition_level = 0;
  column_writer->WriteBatch(1, &definition_level, &repetition_level,
                            &values[3]);

  definition_level = 1;
  repetition_level = 1;
  column_writer->WriteBatch(1, &definition_level, &repetition_level,
                            &values[4]);
  column_writer->WriteBatch(1, &definition_level, &repetition_level,
                            &values[5]);

  file_writer->Close();
  arrow::Status arrow_status = output_file->Close();
  LOG_IF(FATAL, !arrow_status.ok()) << arrow_status.ToString();

  return 0;
}
