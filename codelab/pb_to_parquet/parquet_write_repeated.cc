#include <array>
#include <memory>
#include <string>

#include "absl/status/status.h"
#include "arrow/api.h"
#include "arrow/io/api.h"
#include "arrow/io/file.h"
#include "gflags/gflags.h"
#include "glog/logging.h"
#include "parquet/api/io.h"
#include "parquet/api/schema.h"
#include "parquet/api/writer.h"
#include "parquet/arrow/writer.h"

#define ARROW_CHECK_OK(status)                                        \
  do {                                                                \
    ::arrow::Status __s = ::arrow::internal::GenericToStatus(status); \
    CHECK(__s.ok()) << __s.ToString();                                \
  } while (false)

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  // Parquet-CLI cannot read repeated field directly. This is a by-design
  // behaviour. See further details in
  // https://github.com/apache/parquet-mr/blob/apache-parquet-1.11.0/parquet-protobuf/src/main/java/org/apache/parquet/proto/ProtoMessageConverter.java#L382
  std::shared_ptr<arrow::Schema> schema = arrow::schema(
      {arrow::field("my_int32_array", arrow::list(arrow::int32()))});

  arrow::MemoryPool* pool = arrow::default_memory_pool();
  arrow::ListBuilder list_builder(pool,
                                  std::make_shared<arrow::Int32Builder>(pool));
  arrow::Int32Builder* list_inner_builder =
      dynamic_cast<arrow::Int32Builder*>(list_builder.value_builder());
  ARROW_CHECK_OK(list_builder.Append());
  ARROW_CHECK_OK(list_inner_builder->Append(2));
  ARROW_CHECK_OK(list_inner_builder->Append(3));
  ARROW_CHECK_OK(list_inner_builder->Append(5));
  ARROW_CHECK_OK(list_builder.Append());
  ARROW_CHECK_OK(list_inner_builder->Append(7));
  ARROW_CHECK_OK(list_inner_builder->Append(9));
  ARROW_CHECK_OK(list_inner_builder->Append(11));
  ARROW_CHECK_OK(list_inner_builder->Append(13));
  ARROW_CHECK_OK(list_inner_builder->Append(17));
  ARROW_CHECK_OK(list_inner_builder->Append(19));

  std::shared_ptr<arrow::Array> my_int32_array_values;
  ARROW_CHECK_OK(list_builder.Finish(&my_int32_array_values));

  std::shared_ptr<arrow::Table> table =
      arrow::Table::Make(schema, {my_int32_array_values});
  LOG(INFO) << table->ToString();

  std::shared_ptr<arrow::io::FileOutputStream> output_file =
      arrow::io::FileOutputStream::Open("test.bin").ValueOrDie();
  ARROW_CHECK_OK(parquet::arrow::WriteTable(*table, pool, output_file, 2));

  LOG(INFO) << "DONE";

  // See further details about definition level & repetition level in
  // https://blog.twitter.com/engineering/en_us/a/2013/dremel-made-simple-with-parquet.html

  // parquet::schema::NodeVector fields;
  // fields.emplace_back(parquet::schema::PrimitiveNode::Make(
  //     "my_int32_array", parquet::Repetition::REPEATED,
  //     parquet::LogicalType::None(), parquet::Type::INT32, -1, 1));

  // parquet::schema::NodeVector subfields2;
  // subfields2.emplace_back(parquet::schema::PrimitiveNode::Make(
  //     "element", parquet::Repetition::OPTIONAL, parquet::LogicalType::None(),
  //     parquet::Type::INT32, -1, 1));
  // parquet::schema::NodeVector subfields1;
  // subfields1.emplace_back(parquet::schema::GroupNode::Make(
  //     "list", parquet::Repetition::REPEATED, subfields2, nullptr, 1));
  // fields.emplace_back(parquet::schema::GroupNode::Make(
  //     "my_int32_array", parquet::Repetition::OPTIONAL, subfields1,
  //     parquet::LogicalType::List(), 1));

  // std::shared_ptr<arrow::io::FileOutputStream> output_file =
  //     arrow::io::FileOutputStream::Open("test.bin").ValueOrDie();
  // std::unique_ptr<parquet::ParquetFileWriter> file_writer =
  //     parquet::ParquetFileWriter::Open(
  //         output_file,
  //         std::static_pointer_cast<parquet::schema::GroupNode>(
  //             parquet::schema::GroupNode::Make(
  //                 "__schema", parquet::Repetition::REQUIRED, fields)));
  // CHECK(file_writer);

  // parquet::RowGroupWriter* row_group_writer = file_writer->AppendRowGroup(2);
  // CHECK(row_group_writer);

  // parquet::Int32Writer* column_writer =
  //     static_cast<parquet::Int32Writer*>(row_group_writer->NextColumn());
  // CHECK(column_writer);

  // LOG(INFO) << column_writer->descr()->ToString();
  // std::array<int32_t, 6> values = {17, 19, 23, 27, 31, 37};
  // int16_t definition_level = 1;
  // int16_t repetition_level = 0;
  // column_writer->WriteBatch(1, &definition_level, &repetition_level,
  //                           &values[0]);

  // definition_level = 1;
  // repetition_level = 1;
  // column_writer->WriteBatch(1, &definition_level, &repetition_level,
  //                           &values[1]);
  // column_writer->WriteBatch(1, &definition_level, &repetition_level,
  //                           &values[2]);

  // definition_level = 1;
  // repetition_level = 0;
  // column_writer->WriteBatch(1, &definition_level, &repetition_level,
  //                           &values[3]);

  // definition_level = 1;
  // repetition_level = 1;
  // column_writer->WriteBatch(1, &definition_level, &repetition_level,
  //                           &values[4]);
  // column_writer->WriteBatch(1, &definition_level, &repetition_level,
  //                           &values[5]);

  // file_writer->Close();
  // arrow::Status arrow_status = output_file->Close();
  // LOG_IF(FATAL, !arrow_status.ok()) << arrow_status.ToString();

  return 0;
}
