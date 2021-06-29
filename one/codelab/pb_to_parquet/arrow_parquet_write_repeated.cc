#include <algorithm>
#include <array>
#include <memory>
#include <string>

#include "gflags/gflags.h"
#include "glog/logging.h"
#include "third_party/absl/status/status.h"
#include "third_party/arrow/src/arrow/api.h"
#include "third_party/arrow/src/arrow/io/api.h"
#include "third_party/arrow/src/arrow/io/file.h"
#include "third_party/arrow/src/parquet/api/io.h"
#include "third_party/arrow/src/parquet/api/schema.h"
#include "third_party/arrow/src/parquet/api/writer.h"
#include "third_party/arrow/src/parquet/arrow/writer.h"
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

DEFINE_bool(use_write_table, true, "Use WriteTable API.");

DEFINE_string(output, "arrow_test.parquet", "Output file.");
DEFINE_validator(output, &FlagStringNotEmpty);

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
      arrow::io::FileOutputStream::Open(FLAGS_output).ValueOrDie();

  // Expand following calls for writing multiple row groups.
  if (FLAGS_use_write_table) {
    ARROW_CHECK_OK(parquet::arrow::WriteTable(*table, pool, output_file, 2));
  } else {
    std::unique_ptr<parquet::arrow::FileWriter> file_writer;
    ARROW_CHECK_OK(parquet::arrow::FileWriter::Open(
        *schema, pool, output_file, parquet::default_writer_properties(),
        parquet::default_arrow_writer_properties(), &file_writer));

    int64_t chunk_size = 2;
    ARROW_CHECK_OK(table->Validate());

    CHECK(!(chunk_size <= 0 && table->num_rows() > 0))
        << "chunk size per row_group must be greater than 0";
    CHECK(table->schema()->Equals(*schema, false))
        << "table schema does not match this writer's. table:'"
        << table->schema()->ToString() << "' this:'" << schema->ToString()
        << "'";

    auto WriteRowGroup = [&](int64_t offset, int64_t size) {
      ARROW_CHECK_OK(file_writer->NewRowGroup(size));
      for (int i = 0; i < table->num_columns(); i++) {
        ARROW_CHECK_OK(
            file_writer->WriteColumnChunk(table->column(i), offset, size));
      }
      return arrow::Status::OK();
    };

    if (table->num_rows() == 0) {
      // Append a row group with 0 rows
      ARROW_CHECK_OK(WriteRowGroup(0, 0));
    } else {
      for (int chunk = 0; chunk * chunk_size < table->num_rows(); chunk++) {
        int64_t offset = chunk * chunk_size;
        ARROW_CHECK_OK(WriteRowGroup(
            offset, std::min(chunk_size, table->num_rows() - offset)));
      }
    }

    if (table->num_rows() == 0) {
      // Append a row group with 0 rows
      ARROW_CHECK_OK(WriteRowGroup(0, 0));
    } else {
      for (int chunk = 0; chunk * chunk_size < table->num_rows(); chunk++) {
        int64_t offset = chunk * chunk_size;
        ARROW_CHECK_OK(WriteRowGroup(
            offset, std::min(chunk_size, table->num_rows() - offset)));
      }
    }

    ARROW_CHECK_OK(file_writer->Close());
  }
  ARROW_CHECK_OK(output_file->Close());

  LOG(INFO) << "DONE";

  return 0;
}
