#include "codelab/feature_store/benchmark_encoding/dump.h"

#include <algorithm>
#include <limits>
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
#include "codelab/feature_store/benchmark_encoding/null_output_stream.h"
#include "codelab/feature_store/benchmark_encoding/row.h"
#include "codelab/pb_to_arrow/status_util.h"
#include "status/status_util.h"
#include "util/casts.h"

namespace hcoona {
namespace codelab {
namespace feature_store {

namespace {

constexpr const size_t kBatchSize = 64;
using underlying_bitmap_t = uint64_t;
static_assert(kBatchSize == sizeof(underlying_bitmap_t) * 8);
constexpr const underlying_bitmap_t kLeftMostMask =
    static_cast<underlying_bitmap_t>(1U) << (kBatchSize - 1);
static_assert(kLeftMostMask + kLeftMostMask == 0);

struct ParquetFieldContext {
  size_t column_index;
  const FieldDescriptor* field_descriptor;

  ParquetFieldContext() = default;
  ParquetFieldContext(size_t column_index,
                      const FieldDescriptor* field_descriptor)
      : column_index(column_index), field_descriptor(field_descriptor) {}
};

struct ArrowFieldContext {
  size_t column_index;
  const FieldDescriptor* field_descriptor;
  arrow::ArrayBuilder* array_builder;

  ArrowFieldContext() = default;
  ArrowFieldContext(size_t column_index,
                    const FieldDescriptor* field_descriptor)
      : column_index(column_index), field_descriptor(field_descriptor) {}
  ArrowFieldContext(size_t column_index,
                    const FieldDescriptor* field_descriptor,
                    arrow::ArrayBuilder* array_builder)
      : column_index(column_index),
        field_descriptor(field_descriptor),
        array_builder(array_builder) {}
};

arrow::Compression::type ConvertArrow(CompressionMode mode) {
  switch (mode) {
    case CompressionMode::kNoCompression:
      return arrow::Compression::UNCOMPRESSED;
    case CompressionMode::kSnappy:
      return arrow::Compression::SNAPPY;
    default:
      CHECK(false) << "Unknown compression mode " << static_cast<int32_t>(mode);
  }
}

absl::Status BuildParquetSchemaFields(
    const std::vector<FieldDescriptor>& fields,
    std::vector<absl::string_view>* all_field_names,
    absl::flat_hash_map<absl::string_view /* field_name */,
                        ParquetFieldContext>* field_index,
    parquet::schema::NodeVector* parquet_fields) {
  size_t column_index = 0;
  for (const FieldDescriptor& field : fields) {
    if (field.type == FieldType::kUnspecified) {
      return absl::UnknownError(
          absl::StrCat("Field type must be specified, name=", field.name));
    } else if (field.type == FieldType::kFeature) {
      auto try_emplace_result =
          field_index->try_emplace(field.name, column_index, &field);
      if (try_emplace_result.second) {
        parquet_fields->emplace_back(parquet::schema::PrimitiveNode::Make(
            field.name, parquet::Repetition::OPTIONAL,
            parquet::Type::BYTE_ARRAY));
        all_field_names->emplace_back(field.name);
        column_index++;
      } else {
        LOG(WARNING) << "Skip feature field '" << field.name
                     << "'(column_index=" << column_index
                     << "), it already occurred at column_index="
                     << try_emplace_result.first->second.column_index << " .";
      }
    } else if (field.type == FieldType::kRawFeature) {
      auto try_emplace_result =
          field_index->try_emplace(field.name, column_index, &field);
      if (try_emplace_result.second) {
        parquet::schema::NodeVector parquet_subfields;
        for (size_t j = 0; j < field.children_num; j++) {
          parquet_subfields.emplace_back(parquet::schema::PrimitiveNode::Make(
              absl::StrCat("_", j), parquet::Repetition::OPTIONAL,
              parquet::Type::BYTE_ARRAY));
        }
        parquet_fields->emplace_back(parquet::schema::GroupNode::Make(
            field.name, parquet::Repetition::OPTIONAL,
            std::move(parquet_subfields)));
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

absl::Status BuildArrowSchemaFields(
    arrow::MemoryPool* pool,
    const std::vector<FieldDescriptor>& field_descriptors,
    std::vector<absl::string_view>* all_field_names,
    absl::flat_hash_map<absl::string_view /* field_name */, ArrowFieldContext>*
        field_index,
    std::vector<std::shared_ptr<arrow::Field>>* fields,
    std::vector<std::shared_ptr<arrow::ArrayBuilder>>* fields_builders) {
  size_t column_index = 0;
  for (const FieldDescriptor& field : field_descriptors) {
    if (field.type == FieldType::kUnspecified) {
      return absl::UnknownError(
          absl::StrCat("Field type must be specified, name=", field.name));
    } else if (field.type == FieldType::kFeature) {
      auto try_emplace_result =
          field_index->try_emplace(field.name, column_index, &field);
      if (try_emplace_result.second) {
        all_field_names->emplace_back(field.name);

        fields->emplace_back(arrow::field(field.name, arrow::binary()));

        fields_builders->emplace_back(
            std::make_shared<arrow::BinaryBuilder>(pool));
        try_emplace_result.first->second.array_builder =
            fields_builders->back().get();

        column_index++;
      } else {
        LOG(WARNING) << "Skip feature field '" << field.name
                     << "'(column_index=" << column_index
                     << "), it already occurred at column_index="
                     << try_emplace_result.first->second.column_index << " .";
      }
    } else if (field.type == FieldType::kRawFeature) {
      auto try_emplace_result =
          field_index->try_emplace(field.name, column_index, &field);
      if (try_emplace_result.second) {
        all_field_names->emplace_back(field.name);

        std::vector<std::shared_ptr<arrow::Field>> arrow_subfields;
        std::vector<std::shared_ptr<arrow::ArrayBuilder>>
            arrow_subfields_builders;
        for (size_t j = 0; j < field.children_num; j++) {
          arrow_subfields.emplace_back(
              arrow::field(absl::StrCat("_", j), arrow::binary()));
          arrow_subfields_builders.emplace_back(
              std::make_shared<arrow::BinaryBuilder>(pool));
        }

        fields->emplace_back(
            arrow::field(field.name, arrow::struct_(arrow_subfields)));

        fields_builders->emplace_back(std::make_shared<arrow::StructBuilder>(
            arrow::struct_(arrow_subfields), pool,
            std::move(arrow_subfields_builders)));
        try_emplace_result.first->second.array_builder =
            fields_builders->back().get();

        column_index++;
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
      // Skip empty raw feature, Parquet disallow struct with no field!
      if (p.second.empty()) continue;

      if (!visited_fields_names.contains(p.first)) {
        fields->emplace_back(p.first, p.second.size());
        visited_fields_names.emplace(p.first);
      }
    }
  }

  return absl::OkStatus();
}

absl::Status DumpWithParquetApi(arrow::MemoryPool* memory_pool,
                                CompressionMode compression_mode,
                                const std::vector<FieldDescriptor>& fields,
                                const std::vector<Row>& rows,
                                std::shared_ptr<arrow::io::OutputStream> sink) {
  std::vector<absl::string_view> all_field_names;
  absl::flat_hash_map<absl::string_view /* field_name */, ParquetFieldContext>
      field_index;
  parquet::schema::NodeVector parquet_fields;
  RETURN_STATUS_IF_NOT_OK(BuildParquetSchemaFields(
      fields, &all_field_names, &field_index, &parquet_fields));

  auto schema = std::static_pointer_cast<parquet::schema::GroupNode>(
      parquet::schema::GroupNode::Make(
          "__schema", parquet::Repetition::REQUIRED, parquet_fields));
  auto properties = parquet::WriterProperties::Builder()
                        .memory_pool(memory_pool)
                        ->compression(ConvertArrow(compression_mode))
                        ->build();
  std::unique_ptr<parquet::ParquetFileWriter> file_writer =
      parquet::ParquetFileWriter::Open(std::move(sink), schema, properties);

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
          down_cast<parquet::ByteArrayWriter*>(
              rg_writer->column(field_index[p.first].column_index));
      int16_t definition_level = 1;
      parquet::ByteArray byte_array(p.second);
      byte_array_writer->WriteBatch(1, &definition_level, nullptr, &byte_array);
    }

    for (const std::pair<std::string /* name */,
                         std::vector<std::string> /* serialized features */>&
             p : row.raw_features()) {
      if (!field_index.contains(p.first)) {
        // TODO(zhangshuai.ustc): Validate it. Could only be happened for raw
        // feature with no feature in it.
        continue;
      }

      unvisited_field_names.erase(p.first);
      size_t base_column_index = field_index[p.first].column_index;
      for (size_t j = 0;
           j < std::min(field_index[p.first].field_descriptor->children_num,
                        p.second.size());
           j++) {
        parquet::ByteArrayWriter* byte_array_writer =
            down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(base_column_index + j));
        int16_t definition_level = 2;
        parquet::ByteArray byte_array(p.second[j]);
        byte_array_writer->WriteBatch(1, &definition_level, nullptr,
                                      &byte_array);
      }

      // Fill null values for padding.
      for (size_t j = p.second.size();
           j < field_index[p.first].field_descriptor->children_num; j++) {
        parquet::ByteArrayWriter* byte_array_writer =
            down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(base_column_index + j));
        int16_t definition_level = 1;
        byte_array_writer->WriteBatch(1, &definition_level, nullptr, nullptr);
      }
    }

    // Fill null values for padding.
    for (absl::string_view field_name : unvisited_field_names) {
      DCHECK(field_index.contains(field_name));
      const ParquetFieldContext& field_context = field_index[field_name];
      if (field_context.field_descriptor->type ==
          FieldType::kFeature) {  // feature
        parquet::ByteArrayWriter* byte_array_writer =
            down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(field_context.column_index));
        int16_t definition_level = 0;
        byte_array_writer->WriteBatch(1, &definition_level, nullptr, nullptr);
      } else if (field_context.field_descriptor->type ==
                 FieldType::kRawFeature) {  // raw feature
        size_t base_column_index = field_context.column_index;
        for (size_t j = 0; j < field_context.field_descriptor->children_num;
             j++) {
          parquet::ByteArrayWriter* byte_array_writer =
              down_cast<parquet::ByteArrayWriter*>(
                  rg_writer->column(base_column_index + j));
          int16_t definition_level = 1;
          byte_array_writer->WriteBatch(1, &definition_level, nullptr, nullptr);
        }
      } else {
        return absl::UnimplementedError(
            "Only implemented for kFeature & kRawFeature.");
      }
    }
  }

  file_writer->Close();

  return absl::OkStatus();
}

absl::Status DumpWithParquetApiV2(
    arrow::MemoryPool* memory_pool, CompressionMode compression_mode,
    const std::vector<FieldDescriptor>& fields, const std::vector<Row>& rows,
    std::shared_ptr<arrow::io::OutputStream> sink) {
  std::vector<absl::string_view> all_field_names;
  absl::flat_hash_map<absl::string_view /* field_name */, ParquetFieldContext>
      field_index;
  parquet::schema::NodeVector parquet_fields;
  RETURN_STATUS_IF_NOT_OK(BuildParquetSchemaFields(
      fields, &all_field_names, &field_index, &parquet_fields));

  auto schema = std::static_pointer_cast<parquet::schema::GroupNode>(
      parquet::schema::GroupNode::Make(
          "__schema", parquet::Repetition::REQUIRED, parquet_fields));
  auto properties = parquet::WriterProperties::Builder()
                        .memory_pool(memory_pool)
                        ->compression(ConvertArrow(compression_mode))
                        ->build();
  std::unique_ptr<parquet::ParquetFileWriter> file_writer =
      parquet::ParquetFileWriter::Open(std::move(sink), schema, properties);

  int16_t def_levels[kBatchSize];
  int16_t rep_levels[kBatchSize];
  underlying_bitmap_t valid_bits = static_cast<underlying_bitmap_t>(0U);
  parquet::ByteArray byte_array_values[kBatchSize];

  parquet::RowGroupWriter* rg_writer = file_writer->AppendBufferedRowGroup();
  for (absl::string_view field_name : all_field_names) {
    DCHECK(field_index.contains(field_name));
    const ParquetFieldContext& field_context = field_index[field_name];

    if (field_context.column_index == 1012) {
      LOG(WARNING) << "column 1012 (field_name=" << field_name << ", type="
                   << (field_context.field_descriptor->type ==
                               FieldType::kFeature
                           ? "Feature"
                           : "RawFeature")
                   << ", children_num="
                   << field_context.field_descriptor->children_num << ")";
    }

    // Loop for raw_feature internal features.
    // Only loop once for feature.
    for (size_t raw_feature_internal_offset = 0;
         raw_feature_internal_offset <
         std::max(static_cast<size_t>(1) /* for feature */,
                  field_context.field_descriptor
                      ->children_num /* for raw_feature */);
         raw_feature_internal_offset++) {
      underlying_bitmap_t mask = kLeftMostMask;
      for (size_t i = 0; i < rows.size(); i += kBatchSize) {
        size_t this_batch_size =
            std::min(kBatchSize, rows.size() - i /* remaining count */);
        for (size_t j = 0; j < this_batch_size; j++) {
          const Row& row = rows[i + j];

          if (field_context.field_descriptor->type ==
              FieldType::kFeature) {  // feature
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
          } else if (field_context.field_descriptor->type ==
                     FieldType::kRawFeature) {  // raw_feature
            if (field_context.field_descriptor->children_num == 0) {
              LOG(FATAL) << "Column index " << field_context.column_index
                         << " has none child!";
            } else if (row.raw_features().contains(field_name)) {
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
          } else {
            return absl::UnimplementedError(
                "Only implemented for kFeature & kRawFeature.");
          }

          mask >>= 1;
        }

        parquet::ByteArrayWriter* byte_array_writer =
            down_cast<parquet::ByteArrayWriter*>(
                rg_writer->column(field_context.column_index));
        byte_array_writer->WriteBatchSpaced(
            this_batch_size, def_levels, rep_levels,
            reinterpret_cast<uint8_t*>(&valid_bits), 0, byte_array_values);
      }
    }
  }

  file_writer->Close();

  return absl::OkStatus();
}

absl::Status DumpWithArrowApi(arrow::MemoryPool* memory_pool,
                              CompressionMode compression_mode,
                              const std::vector<FieldDescriptor>& fields,
                              const std::vector<Row>& rows,
                              std::shared_ptr<arrow::io::OutputStream> sink) {
  std::vector<absl::string_view> all_field_names;
  absl::flat_hash_map<absl::string_view /* field_name */, ArrowFieldContext>
      field_index;
  std::vector<std::shared_ptr<arrow::Field>> arrow_fields;
  std::vector<std::shared_ptr<arrow::ArrayBuilder>> arrow_fields_builders;
  RETURN_STATUS_IF_NOT_OK(BuildArrowSchemaFields(
      memory_pool, fields, &all_field_names, &field_index, &arrow_fields,
      &arrow_fields_builders));

  // Build table schema.
  std::shared_ptr<arrow::Schema> schema = arrow::schema(arrow_fields);
  VLOG(1) << "schema=" << schema->ToString();

  // Build table data.
  for (const Row& row : rows) {
    for (absl::string_view field_name : all_field_names) {
      DCHECK(field_index.contains(field_name));
      const ArrowFieldContext& field_context = field_index[field_name];

      if (field_context.field_descriptor->type ==
          FieldType::kFeature) {  // feature
        auto builder =
            down_cast<arrow::BinaryBuilder*>(field_context.array_builder);
        if (row.features().contains(field_name)) {
          RETURN_STATUS_IF_NOT_OK(FromArrowStatus(
              builder->Append(row.features().find(field_name)->second)));
        } else {
          RETURN_STATUS_IF_NOT_OK(FromArrowStatus(builder->AppendNull()));
        }
      } else if (field_context.field_descriptor->type ==
                 FieldType::kRawFeature) {  // raw_feature
        auto builder =
            down_cast<arrow::StructBuilder*>(field_context.array_builder);
        DCHECK_EQ(builder->num_fields(),
                  field_context.field_descriptor->children_num);
        if (field_context.field_descriptor->children_num == 0) {
          LOG(FATAL) << "Column index " << field_context.column_index
                     << " has none child!";
        } else if (row.raw_features().contains(field_name)) {
          RETURN_STATUS_IF_NOT_OK(FromArrowStatus(builder->Append()));
          for (int i = 0; i < static_cast<int>(
                                  field_context.field_descriptor->children_num);
               i++) {
            auto child_builder =
                down_cast<arrow::BinaryBuilder*>(builder->field_builder(i));
            const std::vector<std::string>& children_serialized_bytes =
                row.raw_features().find(field_name)->second;
            if (i < static_cast<int>(children_serialized_bytes.size())) {
              RETURN_STATUS_IF_NOT_OK(FromArrowStatus(
                  child_builder->Append(children_serialized_bytes[i])));
            } else {
              RETURN_STATUS_IF_NOT_OK(
                  FromArrowStatus(child_builder->AppendNull()));
            }
          }
        } else {
          RETURN_STATUS_IF_NOT_OK(FromArrowStatus(builder->AppendNull()));
          for (int i = 0; i < static_cast<int>(
                                  field_context.field_descriptor->children_num);
               i++) {
            auto child_builder =
                down_cast<arrow::BinaryBuilder*>(builder->field_builder(i));
            RETURN_STATUS_IF_NOT_OK(
                FromArrowStatus(child_builder->AppendNull()));
          }
        }
      } else {
        return absl::UnimplementedError(
            "Only implemented for kFeature & kRawFeature.");
      }
    }
  }

  std::vector<std::shared_ptr<arrow::Array>> arrow_arrays;
  arrow_arrays.reserve(arrow_fields_builders.size());
  for (std::size_t i = 0; i < arrow_fields_builders.size(); i++) {
    std::shared_ptr<arrow::Array> array;
    RETURN_STATUS_IF_NOT_OK(
        FromArrowStatus(arrow_fields_builders[i]->Finish(&array)));
    arrow_arrays.emplace_back(std::move(array));
  }

  std::shared_ptr<arrow::Table> table =
      arrow::Table::Make(schema, arrow_arrays, rows.size());
#ifdef NDEBUG
  RETURN_STATUS_IF_NOT_OK(FromArrowStatus(table->Validate()));
#else
  RETURN_STATUS_IF_NOT_OK(FromArrowStatus(table->ValidateFull()));
#endif  // NDEBUG

  auto properties = parquet::WriterProperties::Builder()
                        .memory_pool(memory_pool)
                        ->compression(ConvertArrow(compression_mode))
                        ->build();
  RETURN_STATUS_IF_NOT_OK(FromArrowStatus(parquet::arrow::WriteTable(
      *table, memory_pool, std::move(sink), std::numeric_limits<int64_t>::max(),
      properties)));

  return absl::OkStatus();
}

}  // namespace feature_store
}  // namespace codelab
}  // namespace hcoona
