#include "codelab/feature_store/benchmark_encoding/dump.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "absl/container/btree_set.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "arrow/api.h"
#include "arrow/io/api.h"
#include "glog/logging.h"
#include "parquet/api/schema.h"
#include "parquet/api/writer.h"
#include "parquet/arrow/writer.h"
#include "codelab/feature_store/benchmark_encoding/row.h"
#include "status/status_util.h"
#include "util/casts.h"

namespace codelab {
namespace feature_store {

namespace {

constexpr const size_t kBatchSize = 64;
using underlying_bitmap_t = uint64_t;
static_assert(kBatchSize == sizeof(underlying_bitmap_t) * 8);
constexpr const underlying_bitmap_t kLeftMostMask =
    static_cast<underlying_bitmap_t>(1U) << (kBatchSize - 1);
static_assert(kLeftMostMask + kLeftMostMask == 0);

struct FieldContext {
  size_t column_index;
  size_t subfields_num;

  FieldContext() = default;
  explicit FieldContext(size_t column_index)
      : column_index(column_index), subfields_num(0) {}
  FieldContext(size_t column_index, size_t subfields_num)
      : column_index(column_index), subfields_num(subfields_num) {}
};

class NullOutputStream : public arrow::io::OutputStream {
 public:
  NullOutputStream() : closed_(false), position_(0) {}
  ~NullOutputStream() override = default;

  /// \brief Close the stream cleanly
  ///
  /// For writable streams, this will attempt to flush any pending data
  /// before releasing the underlying resource.
  ///
  /// After Close() is called, closed() returns true and the stream is not
  /// available for further operations.
  arrow::Status Close() override {
    closed_ = true;
    return arrow::Status::OK();
  }

  /// \brief Close the stream abruptly
  ///
  /// This method does not guarantee that any pending data is flushed.
  /// It merely releases any underlying resource used by the stream for
  /// its operation.
  ///
  /// After Abort() is called, closed() returns true and the stream is not
  /// available for further operations.
  arrow::Status Abort() override {
    closed_ = true;
    return arrow::Status::OK();
  }

  /// \brief Return the position in this stream
  arrow::Result<int64_t> Tell() const override { return position_; }

  /// \brief Return whether the stream is closed
  bool closed() const override { return closed_; }

  /// \brief Write the given data to the stream
  ///
  /// This method always processes the bytes in full.  Depending on the
  /// semantics of the stream, the data may be written out immediately,
  /// held in a buffer, or written asynchronously.  In the case where
  /// the stream buffers the data, it will be copied.  To avoid potentially
  /// large copies, use the Write variant that takes an owned Buffer.
  arrow::Status Write(const void* data, int64_t nbytes) override {
    position_ += nbytes;
    return arrow::Status::OK();
  }

  /// \brief Write the given data to the stream
  ///
  /// Since the Buffer owns its memory, this method can avoid a copy if
  /// buffering is required.  See Write(const void*, int64_t) for details.
  arrow::Status Write(const std::shared_ptr<arrow::Buffer>& data) override {
    position_ += data->size();
    return arrow::Status::OK();
  }

  /// \brief Flush buffered bytes, if any
  arrow::Status Flush() override { return arrow::Status::OK(); }

 private:
  bool closed_;
  int64_t position_;
};

absl::Status BuildParquetSchemaFields(
    const std::vector<FieldDescriptor>& fields,
    std::vector<absl::string_view>* all_field_names,
    absl::flat_hash_map<absl::string_view /* field_name */, FieldContext>*
        field_index,
    parquet::schema::NodeVector* parquet_fields) {
  size_t column_index = 0;
  for (const FieldDescriptor& field : fields) {
    if (field.type == FieldType::kUnspecified) {
      return absl::UnknownError(
          absl::StrCat("Field type must be specified, name=", field.name));
    } else if (field.type == FieldType::kFeature) {
      parquet_fields->emplace_back(parquet::schema::PrimitiveNode::Make(
          field.name, parquet::Repetition::OPTIONAL,
          parquet::Type::BYTE_ARRAY));
      auto try_emplace_result =
          field_index->try_emplace(field.name, column_index);
      if (try_emplace_result.second) {
        all_field_names->emplace_back(field.name);
        column_index++;
      } else {
        LOG(WARNING) << "Skip feature field '" << field.name
                     << "'(column_index=" << column_index
                     << "), it already occurred at column_index="
                     << try_emplace_result.first->second.column_index << " .";
      }
    } else if (field.type == FieldType::kRawFeature) {
      parquet::schema::NodeVector parquet_subfields;
      for (size_t j = 0; j < field.children_num; j++) {
        parquet_subfields.emplace_back(parquet::schema::PrimitiveNode::Make(
            absl::StrCat("_", j), parquet::Repetition::OPTIONAL,
            parquet::Type::BYTE_ARRAY));
      }
      parquet_fields->emplace_back(parquet::schema::GroupNode::Make(
          field.name, parquet::Repetition::OPTIONAL,
          std::move(parquet_subfields)));
      auto try_emplace_result = field_index->try_emplace(
          field.name, column_index, field.children_num);
      if (try_emplace_result.second) {
        all_field_names->emplace_back(field.name);
        column_index += field.children_num;
      } else {
        LOG(WARNING) << "Skip raw feature field '" << field.name
                     << "'(column_index=" << column_index
                     << "), it already occurred at column_index="
                     << try_emplace_result.first->second.column_index << " .";
      }
    } else {
      return absl::UnimplementedError(
          absl::StrCat("Not implemented for field type: ", field.type));
    }
  }

  return absl::OkStatus();
}

}  // namespace

absl::Status GenerateSchema(const std::vector<Row>& rows,
                            std::vector<FieldDescriptor>* fields) {
  absl::flat_hash_set<absl::string_view> visited_fields_names;
  for (const Row& row : rows) {
    for (const std::pair<std::string /* name */,
                         std::string /* serialized bytes */>& p :
         row.features()) {
      if (!visited_fields_names.contains(p.first)) {
        fields->emplace_back(p.first);
        visited_fields_names.emplace(p.first);
      }
    }

    for (const std::pair<std::string /* name */,
                         std::vector<std::string> /* serialized features */>&
             p : row.raw_features()) {
      if (!visited_fields_names.contains(p.first)) {
        fields->emplace_back(p.first, p.second.size());
        visited_fields_names.emplace(p.first);
      }
    }
  }

  return absl::OkStatus();
}

absl::Status DumpWithParquetApi(const std::vector<FieldDescriptor>& fields,
                                const std::vector<Row>& rows,
                                int64_t* written_bytes) {
  std::vector<absl::string_view> all_field_names;
  absl::flat_hash_map<absl::string_view /* field_name */, FieldContext>
      field_index;
  parquet::schema::NodeVector parquet_fields;
  RETURN_STATUS_IF_NOT_OK(BuildParquetSchemaFields(
      fields, &all_field_names, &field_index, &parquet_fields));

  auto schema = std::static_pointer_cast<parquet::schema::GroupNode>(
      parquet::schema::GroupNode::Make(
          "__schema", parquet::Repetition::REQUIRED, parquet_fields));
  auto output_stream = std::make_shared<NullOutputStream>();
  std::unique_ptr<parquet::ParquetFileWriter> file_writer =
      parquet::ParquetFileWriter::Open(output_stream, schema);

  parquet::RowGroupWriter* rg_writer = file_writer->AppendBufferedRowGroup();
  for (const Row& row : rows) {
    absl::btree_set<absl::string_view> unvisited_field_names(
        std::begin(all_field_names), std::end(all_field_names));

    for (const std::pair<std::string /* name */,
                         std::string /* serialized bytes */>& p :
         row.features()) {
      DCHECK(field_index.contains(p.first));
      unvisited_field_names.erase(p.first);
      parquet::ByteArrayWriter* byte_array_writer =
          hcoona::down_cast<parquet::ByteArrayWriter*>(
              rg_writer->column(field_index[p.first].column_index));
      int16_t definition_level = 1;
      parquet::ByteArray byte_array(p.second);
      byte_array_writer->WriteBatch(1, &definition_level, nullptr, &byte_array);
    }

    for (const std::pair<std::string /* name */,
                         std::vector<std::string> /* serialized features */>&
             p : row.raw_features()) {
      DCHECK(field_index.contains(p.first));
      unvisited_field_names.erase(p.first);
      size_t base_column_index = field_index[p.first].column_index;
      for (size_t j = 0;
           j < std::min(field_index[p.first].subfields_num, p.second.size());
           j++) {
        parquet::ByteArrayWriter* byte_array_writer =
            hcoona::down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(base_column_index + j));
        int16_t definition_level = 2;
        parquet::ByteArray byte_array(p.second[j]);
        byte_array_writer->WriteBatch(1, &definition_level, nullptr,
                                      &byte_array);
      }

      // Fill null values for padding.
      for (size_t j = p.second.size(); j < field_index[p.first].subfields_num;
           j++) {
        parquet::ByteArrayWriter* byte_array_writer =
            hcoona::down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(base_column_index + j));
        int16_t definition_level = 1;
        byte_array_writer->WriteBatch(1, &definition_level, nullptr, nullptr);
      }
    }

    // Fill null values for padding.
    for (absl::string_view field_name : unvisited_field_names) {
      DCHECK(field_index.contains(field_name));
      const FieldContext& field_context = field_index[field_name];
      if (field_context.subfields_num == 0) {  // feature
        parquet::ByteArrayWriter* byte_array_writer =
            hcoona::down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(field_context.column_index));
        int16_t definition_level = 0;
        byte_array_writer->WriteBatch(1, &definition_level, nullptr, nullptr);
      } else {  // raw feature
        size_t base_column_index = field_context.column_index;
        for (size_t j = 0; j < field_context.subfields_num; j++) {
          parquet::ByteArrayWriter* byte_array_writer =
              hcoona::down_cast<parquet::ByteArrayWriter*>(
                  rg_writer->column(base_column_index + j));
          int16_t definition_level = 1;
          byte_array_writer->WriteBatch(1, &definition_level, nullptr, nullptr);
        }
      }
    }
  }

  file_writer->Close();
  *written_bytes = output_stream->Tell().ValueOrDie();

  return absl::OkStatus();
}

absl::Status DumpWithParquetApiV2(const std::vector<FieldDescriptor>& fields,
                                  const std::vector<Row>& rows,
                                  int64_t* written_bytes) {
  std::vector<absl::string_view> all_field_names;
  absl::flat_hash_map<absl::string_view /* field_name */, FieldContext>
      field_index;
  parquet::schema::NodeVector parquet_fields;
  RETURN_STATUS_IF_NOT_OK(BuildParquetSchemaFields(
      fields, &all_field_names, &field_index, &parquet_fields));

  auto schema = std::static_pointer_cast<parquet::schema::GroupNode>(
      parquet::schema::GroupNode::Make(
          "__schema", parquet::Repetition::REQUIRED, parquet_fields));
  auto output_stream = std::make_shared<NullOutputStream>();
  std::unique_ptr<parquet::ParquetFileWriter> file_writer =
      parquet::ParquetFileWriter::Open(output_stream, schema);

  int16_t def_levels[kBatchSize];
  int16_t rep_levels[kBatchSize];
  underlying_bitmap_t valid_bits = static_cast<underlying_bitmap_t>(0U);
  parquet::ByteArray byte_array_values[kBatchSize];

  parquet::RowGroupWriter* rg_writer = file_writer->AppendBufferedRowGroup();
  for (absl::string_view field_name : all_field_names) {
    const FieldContext& field_context = field_index[field_name];

    // Loop for raw_feature internal features.
    // Only loop once for feature.
    for (size_t raw_feature_internal_offset = 0;
         raw_feature_internal_offset <
         std::max(static_cast<size_t>(1) /* for feature */,
                  field_context.subfields_num /* for raw_feature */);
         raw_feature_internal_offset++) {
      underlying_bitmap_t mask = kLeftMostMask;
      for (size_t i = 0; i < rows.size(); i += kBatchSize) {
        size_t this_batch_size =
            std::min(kBatchSize, rows.size() - i /* remaining count */);
        for (size_t j = 0; j < this_batch_size; j++) {
          const Row& row = rows[i + j];

          if (field_context.subfields_num == 0) {  // feature
            if (row.features().contains(field_name)) {
              def_levels[j] = 1;
              rep_levels[j] = 0;
              valid_bits |= mask;  // Set mask position

              const std::string& serialized_bytes =
                  row.features().find(field_name)->second;
              byte_array_values[j].ptr =
                  reinterpret_cast<const uint8_t*>(serialized_bytes.data());
              byte_array_values[j].len = serialized_bytes.size();
            } else {
              def_levels[j] = 0;
              rep_levels[j] = 0;
              valid_bits &= ~mask;  // Clear mask position
            }
          } else {  // raw_feature
            if (row.raw_features().contains(field_name)) {
              def_levels[j] = 2;
              rep_levels[j] = 0;
              valid_bits |= mask;  // Set mask position

              const std::string& serialized_bytes =
                  row.raw_features()
                      .find(field_name)
                      ->second[raw_feature_internal_offset];
              byte_array_values[j].ptr =
                  reinterpret_cast<const uint8_t*>(serialized_bytes.data());
              byte_array_values[j].len = serialized_bytes.size();
            } else {
              def_levels[j] = 1;
              rep_levels[j] = 0;
              valid_bits &= ~mask;  // Clear mask position
            }
          }

          mask >>= 1;
        }

        parquet::ByteArrayWriter* byte_array_writer =
            hcoona::down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(field_context.column_index));
        byte_array_writer->WriteBatchSpaced(
            this_batch_size, def_levels, rep_levels,
            reinterpret_cast<uint8_t*>(&valid_bits), 0, byte_array_values);
      }
    }
  }

  file_writer->Close();
  *written_bytes = output_stream->Tell().ValueOrDie();

  return absl::OkStatus();
}

absl::Status DumpWithArrowApi(arrow::MemoryPool* memory_pool,
                              const std::vector<FieldDescriptor>& fields,
                              const std::vector<Row>& rows,
                              int64_t* written_bytes) {
  return absl::UnimplementedError("");
}

}  // namespace feature_store
}  // namespace codelab
