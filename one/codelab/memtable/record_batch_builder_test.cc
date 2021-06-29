#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "third_party/arrow/src/arrow/api.h"
#include "one/base/casts.h"
#include "one/codelab/pb_to_arrow/status_util.h"
#include "gtl/no_destructor.h"

namespace hcoona {
namespace codelab {
namespace {

constexpr const size_t PARTITION_ID_COL_INDEX = 0;
constexpr const char PARTITION_ID_COL_NAME[] = "__partition_id";
constexpr const size_t OFFSET_COL_INDEX = 1;
constexpr const char OFFSET_COL_NAME[] = "__offset";
constexpr const size_t TIMESTAMP_COL_INDEX = 2;
constexpr const char TIMESTAMP_COL_NAME[] = "__timestamp";

const std::vector<std::shared_ptr<arrow::Field>>& GetBuiltinFields() {
  static gtl::NoDestructor<std::vector<std::shared_ptr<arrow::Field>>>
      builtin_fields([]() {
        std::vector<std::shared_ptr<arrow::Field>> fields;
        fields.emplace_back(arrow::field(PARTITION_ID_COL_NAME, arrow::int32(),
                                         false /* nullable */));
        fields.emplace_back(arrow::field(OFFSET_COL_NAME, arrow::int64(),
                                         false /* nullable */));
        fields.emplace_back(arrow::field(TIMESTAMP_COL_NAME, arrow::int64(),
                                         false /* nullable */));
        return fields;
      }());
  return *builtin_fields;
}

const std::shared_ptr<arrow::Schema>& GetBuiltinFieldsSchema() {
  static gtl::NoDestructor<std::shared_ptr<arrow::Schema>> schema([]() {
    std::shared_ptr<arrow::Schema> schema;

    std::vector<std::shared_ptr<arrow::Field>> fields;
    const std::vector<std::shared_ptr<arrow::Field>>& builtin_fields =
        GetBuiltinFields();
    fields.insert(fields.begin(), builtin_fields.begin(), builtin_fields.end());

    schema = arrow::schema(std::move(fields));
    return schema;
  }());

  return *schema;
}

absl::Status Append(arrow::RecordBatchBuilder* builder, int32_t partition_id,
                    int64_t offset, int64_t timestamp) {
  absl::Status s = FromArrowStatus(
      builder->GetFieldAs<arrow::Int32Builder>(PARTITION_ID_COL_INDEX)
          ->Append(static_cast<int32_t>(partition_id)));
  if (!s.ok()) {
    return s;
  }

  s = FromArrowStatus(builder->GetFieldAs<arrow::Int64Builder>(OFFSET_COL_INDEX)
                          ->Append(offset));
  if (!s.ok()) {
    return s;
  }

  s = FromArrowStatus(
      builder->GetFieldAs<arrow::Int64Builder>(TIMESTAMP_COL_INDEX)
          ->Append(timestamp));
  if (!s.ok()) {
    return s;
  }

  return absl::OkStatus();
}

int64_t ReadFirstOffset(const arrow::Table& table) {
  DCHECK_EQ(table.field(OFFSET_COL_INDEX)->name(), OFFSET_COL_NAME);
  DCHECK_GT(table.num_rows(), 0);

  std::shared_ptr<arrow::Array> array =
      table.column(OFFSET_COL_INDEX)->chunk(0);
  return down_cast<arrow::Int64Array*>(array.get())->Value(0);
}

int64_t ReadLastOffset(const arrow::Table& table) {
  DCHECK_EQ(table.field(OFFSET_COL_INDEX)->name(), OFFSET_COL_NAME);
  DCHECK_GT(table.num_rows(), 0);

  std::shared_ptr<arrow::ChunkedArray> chunked_array =
      table.column(OFFSET_COL_INDEX);
  std::shared_ptr<arrow::Array> array =
      chunked_array->chunk(chunked_array->num_chunks() - 1);
  return down_cast<arrow::Int64Array*>(array.get())->Value(array->length() - 1);
}

int64_t ReadLatestTimestamp(const arrow::Table& table) {
  DCHECK_EQ(table.field(TIMESTAMP_COL_INDEX)->name(), TIMESTAMP_COL_NAME);
  DCHECK_GT(table.num_rows(), 0);

  std::shared_ptr<arrow::ChunkedArray> chunked_array =
      table.column(TIMESTAMP_COL_INDEX);

  int64_t t = -1;
  for (int i = 0; i < chunked_array->num_chunks(); i++) {
    std::shared_ptr<arrow::Array> array = chunked_array->chunk(i);
    auto* int64_array = down_cast<arrow::Int64Array*>(array.get());
    for (int j = 0; j < int64_array->length(); j++) {
      t = std::max(t, int64_array->Value(j));
    }
  }
  return t;
}

TEST(TestRecordBatchBuilder, TestWriteThenReadFirst) {
  std::unique_ptr<arrow::RecordBatchBuilder> builder;
  absl::Status s = FromArrowStatus(arrow::RecordBatchBuilder::Make(
      GetBuiltinFieldsSchema(), arrow::default_memory_pool(), &builder));
  ASSERT_TRUE(s.ok()) << s.ToString();

  s = Append(builder.get(), 0, 1, 0);
  ASSERT_TRUE(s.ok()) << s.ToString();

  int64_t first_offset =
      builder->GetFieldAs<arrow::Int64Builder>(OFFSET_COL_INDEX)->GetValue(0);
  EXPECT_EQ(first_offset, 1);
}

TEST(TestRecordBatch, TestWriteThenReadFirst) {
  std::unique_ptr<arrow::RecordBatchBuilder> builder;
  absl::Status s = FromArrowStatus(arrow::RecordBatchBuilder::Make(
      GetBuiltinFieldsSchema(), arrow::default_memory_pool(), &builder));
  ASSERT_TRUE(s.ok()) << s.ToString();

  s = Append(builder.get(), 0, 1, 0);
  ASSERT_TRUE(s.ok()) << s.ToString();

  std::shared_ptr<arrow::RecordBatch> record_batch;
  s = FromArrowStatus(builder->Flush(true, &record_batch));
  ASSERT_TRUE(s.ok()) << s.ToString();

  ASSERT_EQ(record_batch->num_rows(), 1);

  int64_t first_offset = std::dynamic_pointer_cast<arrow::Int64Array>(
                             record_batch->column(OFFSET_COL_INDEX))
                             ->Value(0);
  EXPECT_EQ(first_offset, 1);
}

TEST(TestArrowTable, TestWriteThenReadFirst) {
  std::unique_ptr<arrow::RecordBatchBuilder> builder;
  absl::Status s = FromArrowStatus(arrow::RecordBatchBuilder::Make(
      GetBuiltinFieldsSchema(), arrow::default_memory_pool(), &builder));
  ASSERT_TRUE(s.ok()) << s.ToString();

  s = Append(builder.get(), 0, 1, 19);
  ASSERT_TRUE(s.ok()) << s.ToString();

  s = Append(builder.get(), 0, 2, 17);
  ASSERT_TRUE(s.ok()) << s.ToString();

  std::shared_ptr<arrow::RecordBatch> record_batch;
  s = FromArrowStatus(builder->Flush(true, &record_batch));
  ASSERT_TRUE(s.ok()) << s.ToString();

  std::vector<std::shared_ptr<arrow::RecordBatch>> record_batches;
  record_batches.emplace_back(std::move(record_batch));

  arrow::Result<std::shared_ptr<arrow::Table>> result =
      arrow::Table::FromRecordBatches(record_batches);
  ASSERT_TRUE(result.ok()) << result.status().ToString();

  std::shared_ptr<arrow::Table> table = result.MoveValueUnsafe()->Slice(0, 2);
  s = FromArrowStatus(table->ValidateFull());
  ASSERT_TRUE(s.ok()) << s.ToString();
  ASSERT_EQ(table->num_rows(), 2);

  EXPECT_EQ(ReadFirstOffset(*table), 1);
  EXPECT_EQ(ReadLastOffset(*table), 2);
  EXPECT_EQ(ReadLatestTimestamp(*table), 19);
}

}  // namespace
}  // namespace codelab
}  // namespace hcoona
