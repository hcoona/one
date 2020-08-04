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
#include "arrow/io/api.h"
#include "glog/logging.h"
#include "parquet/api/schema.h"
#include "parquet/api/writer.h"
#include "codelab/feature_store/benchmark_encoding/row.h"
#include "status/status_util.h"
#include "util/casts.h"

namespace codelab {
namespace feature_store {

namespace {

struct FieldContext {
  size_t column_index;
  size_t subfields_num;

  FieldContext() = default;
  explicit FieldContext(size_t column_index)
      : column_index(column_index), subfields_num(0) {}
  FieldContext(size_t column_index, size_t subfields_num)
      : column_index(column_index), subfields_num(subfields_num) {}
};

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

absl::Status DumpWithParquetApi(const std::vector<FieldDescriptor>& fields,
                                const std::vector<Row>& rows) {
  std::vector<absl::string_view> all_field_names;
  absl::flat_hash_map<absl::string_view /* field_name */, FieldContext>
      field_index;
  parquet::schema::NodeVector parquet_fields;
  RETURN_STATUS_IF_NOT_OK(BuildParquetSchemaFields(
      fields, &all_field_names, &field_index, &parquet_fields));

  auto schema = std::static_pointer_cast<parquet::schema::GroupNode>(
      parquet::schema::GroupNode::Make(
          "__schema", parquet::Repetition::REQUIRED, parquet_fields));
  std::unique_ptr<parquet::ParquetFileWriter> file_writer =
      parquet::ParquetFileWriter::Open(
          arrow::io::FileOutputStream::Open("/dev/null").ValueOrDie(), schema);

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

  return absl::OkStatus();
}

absl::Status DumpWithParquetApiV2(const std::vector<Row>& rows) {
  return absl::UnimplementedError("");
}

absl::Status DumpWithArrowApi(const std::vector<Row>& rows) {
  return absl::UnimplementedError("");
}

}  // namespace feature_store
}  // namespace codelab
