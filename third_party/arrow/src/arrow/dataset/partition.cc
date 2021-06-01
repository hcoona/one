// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "third_party/arrow/src/arrow/dataset/partition.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "third_party/arrow/src/arrow/array/array_base.h"
#include "third_party/arrow/src/arrow/array/array_dict.h"
#include "third_party/arrow/src/arrow/array/array_nested.h"
#include "third_party/arrow/src/arrow/array/builder_dict.h"
#include "third_party/arrow/src/arrow/compute/api_aggregate.h"
#include "third_party/arrow/src/arrow/compute/api_scalar.h"
#include "third_party/arrow/src/arrow/compute/api_vector.h"
#include "third_party/arrow/src/arrow/compute/cast.h"
#include "third_party/arrow/src/arrow/dataset/dataset_internal.h"
#include "third_party/arrow/src/arrow/filesystem/path_util.h"
#include "third_party/arrow/src/arrow/scalar.h"
#include "third_party/arrow/src/arrow/util/int_util_internal.h"
#include "third_party/arrow/src/arrow/util/logging.h"
#include "third_party/arrow/src/arrow/util/make_unique.h"
#include "third_party/arrow/src/arrow/util/string_view.h"

namespace arrow {

using internal::checked_cast;
using internal::checked_pointer_cast;
using util::string_view;

namespace dataset {

std::shared_ptr<Partitioning> Partitioning::Default() {
  class DefaultPartitioning : public Partitioning {
   public:
    DefaultPartitioning() : Partitioning(::arrow::schema({})) {}

    std::string type_name() const override { return "default"; }

    Result<Expression> Parse(const std::string& path) const override {
      return literal(true);
    }

    Result<std::string> Format(const Expression& expr) const override {
      return Status::NotImplemented("formatting paths from ", type_name(),
                                    " Partitioning");
    }

    Result<PartitionedBatches> Partition(
        const std::shared_ptr<RecordBatch>& batch) const override {
      return PartitionedBatches{{batch}, {literal(true)}};
    }
  };

  return std::make_shared<DefaultPartitioning>();
}

static Result<RecordBatchVector> ApplyGroupings(
    const ListArray& groupings, const std::shared_ptr<RecordBatch>& batch) {
  ARROW_ASSIGN_OR_RAISE(Datum sorted,
                        compute::Take(batch, groupings.data()->child_data[0]));

  const auto& sorted_batch = *sorted.record_batch();

  RecordBatchVector out(static_cast<size_t>(groupings.length()));
  for (size_t i = 0; i < out.size(); ++i) {
    out[i] = sorted_batch.Slice(groupings.value_offset(i), groupings.value_length(i));
  }

  return out;
}

Result<Partitioning::PartitionedBatches> KeyValuePartitioning::Partition(
    const std::shared_ptr<RecordBatch>& batch) const {
  std::vector<int> key_indices;
  int num_keys = 0;

  // assemble vector of indices of fields in batch on which we'll partition
  for (const auto& partition_field : schema_->fields()) {
    ARROW_ASSIGN_OR_RAISE(
        auto match, FieldRef(partition_field->name()).FindOneOrNone(*batch->schema()))

    if (match.empty()) continue;
    key_indices.push_back(match[0]);
    ++num_keys;
  }

  if (key_indices.empty()) {
    // no fields to group by; return the whole batch
    return PartitionedBatches{{batch}, {literal(true)}};
  }

  // assemble an ExecBatch of the key columns
  compute::ExecBatch key_batch({}, batch->num_rows());
  for (int i : key_indices) {
    key_batch.values.emplace_back(batch->column_data(i));
  }

  ARROW_ASSIGN_OR_RAISE(auto grouper,
                        compute::internal::Grouper::Make(key_batch.GetDescriptors()));

  ARROW_ASSIGN_OR_RAISE(Datum id_batch, grouper->Consume(key_batch));

  auto ids = id_batch.array_as<UInt32Array>();
  ARROW_ASSIGN_OR_RAISE(auto groupings, compute::internal::Grouper::MakeGroupings(
                                            *ids, grouper->num_groups()));

  ARROW_ASSIGN_OR_RAISE(auto uniques, grouper->GetUniques());
  ArrayVector unique_arrays(num_keys);
  for (int i = 0; i < num_keys; ++i) {
    unique_arrays[i] = uniques.values[i].make_array();
  }

  PartitionedBatches out;

  // assemble partition expressions from the unique keys
  out.expressions.resize(grouper->num_groups());
  for (uint32_t group = 0; group < grouper->num_groups(); ++group) {
    std::vector<Expression> exprs(num_keys);

    for (int i = 0; i < num_keys; ++i) {
      ARROW_ASSIGN_OR_RAISE(auto val, unique_arrays[i]->GetScalar(group));
      const auto& name = batch->schema()->field(key_indices[i])->name();

      exprs[i] = val->is_valid ? equal(field_ref(name), literal(std::move(val)))
                               : is_null(field_ref(name));
    }
    out.expressions[group] = and_(std::move(exprs));
  }

  // remove key columns from batch to which we'll be applying the groupings
  auto rest = batch;
  std::sort(key_indices.begin(), key_indices.end(), std::greater<int>());
  for (int i : key_indices) {
    // indices are in descending order; indices larger than i (which would be invalidated
    // here) have already been handled
    ARROW_ASSIGN_OR_RAISE(rest, rest->RemoveColumn(i));
  }
  ARROW_ASSIGN_OR_RAISE(out.batches, ApplyGroupings(*groupings, rest));

  return out;
}

Result<Expression> KeyValuePartitioning::ConvertKey(const Key& key) const {
  ARROW_ASSIGN_OR_RAISE(auto match, FieldRef(key.name).FindOneOrNone(*schema_));
  if (match.empty()) {
    return literal(true);
  }

  auto field_index = match[0];
  auto field = schema_->field(field_index);

  std::shared_ptr<Scalar> converted;

  if (!key.value.has_value()) {
    return is_null(field_ref(field->name()));
  } else if (field->type()->id() == Type::DICTIONARY) {
    if (dictionaries_.empty() || dictionaries_[field_index] == nullptr) {
      return Status::Invalid("No dictionary provided for dictionary field ",
                             field->ToString());
    }

    DictionaryScalar::ValueType value;
    value.dictionary = dictionaries_[field_index];

    const auto& dictionary_type = checked_cast<const DictionaryType&>(*field->type());
    if (!value.dictionary->type()->Equals(dictionary_type.value_type())) {
      return Status::TypeError("Dictionary supplied for field ", field->ToString(),
                               " had incorrect type ",
                               value.dictionary->type()->ToString());
    }

    // look up the partition value in the dictionary
    ARROW_ASSIGN_OR_RAISE(converted, Scalar::Parse(value.dictionary->type(), *key.value));
    ARROW_ASSIGN_OR_RAISE(auto index, compute::IndexIn(converted, value.dictionary));
    auto to_index_type = compute::CastOptions::Safe(dictionary_type.index_type());
    ARROW_ASSIGN_OR_RAISE(index, compute::Cast(index, to_index_type));
    value.index = index.scalar();
    if (!value.index->is_valid) {
      return Status::Invalid("Dictionary supplied for field ", field->ToString(),
                             " does not contain '", *key.value, "'");
    }
    converted = std::make_shared<DictionaryScalar>(std::move(value), field->type());
  } else {
    ARROW_ASSIGN_OR_RAISE(converted, Scalar::Parse(field->type(), *key.value));
  }

  return equal(field_ref(field->name()), literal(std::move(converted)));
}

Result<Expression> KeyValuePartitioning::Parse(const std::string& path) const {
  std::vector<Expression> expressions;

  for (const Key& key : ParseKeys(path)) {
    ARROW_ASSIGN_OR_RAISE(auto expr, ConvertKey(key));
    if (expr == literal(true)) continue;
    expressions.push_back(std::move(expr));
  }

  return and_(std::move(expressions));
}

Result<std::string> KeyValuePartitioning::Format(const Expression& expr) const {
  ScalarVector values{static_cast<size_t>(schema_->num_fields()), nullptr};

  ARROW_ASSIGN_OR_RAISE(auto known_values, ExtractKnownFieldValues(expr));
  for (const auto& ref_value : known_values) {
    if (!ref_value.second.is_scalar()) {
      return Status::Invalid("non-scalar partition key ", ref_value.second.ToString());
    }

    ARROW_ASSIGN_OR_RAISE(auto match, ref_value.first.FindOneOrNone(*schema_));
    if (match.empty()) continue;

    auto value = ref_value.second.scalar();

    const auto& field = schema_->field(match[0]);
    if (!value->type->Equals(field->type())) {
      if (value->is_valid) {
        auto maybe_converted = compute::Cast(value, field->type());
        if (!maybe_converted.ok()) {
          return Status::TypeError("Error converting scalar ", value->ToString(),
                                   " (of type ", *value->type,
                                   ") to a partition key for ", field->ToString(), ": ",
                                   maybe_converted.status().message());
        }
        value = maybe_converted->scalar();
      } else {
        value = MakeNullScalar(field->type());
      }
    }

    if (value->type->id() == Type::DICTIONARY) {
      ARROW_ASSIGN_OR_RAISE(
          value, checked_cast<const DictionaryScalar&>(*value).GetEncodedValue());
    }

    values[match[0]] = std::move(value);
  }

  return FormatValues(values);
}

std::vector<KeyValuePartitioning::Key> DirectoryPartitioning::ParseKeys(
    const std::string& path) const {
  std::vector<Key> keys;

  int i = 0;
  for (auto&& segment : fs::internal::SplitAbstractPath(path)) {
    if (i >= schema_->num_fields()) break;

    keys.push_back({schema_->field(i++)->name(), std::move(segment)});
  }

  return keys;
}

inline util::optional<int> NextValid(const ScalarVector& values, int first_null) {
  auto it = std::find_if(values.begin() + first_null + 1, values.end(),
                         [](const std::shared_ptr<Scalar>& v) { return v != nullptr; });

  if (it == values.end()) {
    return util::nullopt;
  }

  return static_cast<int>(it - values.begin());
}

Result<std::string> DirectoryPartitioning::FormatValues(
    const ScalarVector& values) const {
  std::vector<std::string> segments(static_cast<size_t>(schema_->num_fields()));

  for (int i = 0; i < schema_->num_fields(); ++i) {
    if (values[i] != nullptr && values[i]->is_valid) {
      segments[i] = values[i]->ToString();
      continue;
    }

    if (auto illegal_index = NextValid(values, i)) {
      // XXX maybe we should just ignore keys provided after the first absent one?
      return Status::Invalid("No partition key for ", schema_->field(i)->name(),
                             " but a key was provided subsequently for ",
                             schema_->field(*illegal_index)->name(), ".");
    }

    // if all subsequent keys are absent we'll just print the available keys
    break;
  }

  return fs::internal::JoinAbstractPath(std::move(segments));
}

namespace {
class KeyValuePartitioningFactory : public PartitioningFactory {
 protected:
  explicit KeyValuePartitioningFactory(PartitioningFactoryOptions options)
      : options_(std::move(options)) {}

  int GetOrInsertField(const std::string& name) {
    auto it_inserted =
        name_to_index_.emplace(name, static_cast<int>(name_to_index_.size()));

    if (it_inserted.second) {
      repr_memos_.push_back(MakeMemo());
    }

    return it_inserted.first->second;
  }

  Status InsertRepr(const std::string& name, util::optional<string_view> repr) {
    auto field_index = GetOrInsertField(name);
    if (repr.has_value()) {
      return InsertRepr(field_index, *repr);
    } else {
      return Status::OK();
    }
  }

  Status InsertRepr(int index, util::string_view repr) {
    int dummy;
    return repr_memos_[index]->GetOrInsert<StringType>(repr, &dummy);
  }

  Result<std::shared_ptr<Schema>> DoInspect() {
    dictionaries_.assign(name_to_index_.size(), nullptr);

    std::vector<std::shared_ptr<Field>> fields(name_to_index_.size());
    if (options_.schema) {
      const auto requested_size = options_.schema->fields().size();
      const auto inferred_size = fields.size();
      if (inferred_size != requested_size) {
        return Status::Invalid("Requested schema has ", requested_size,
                               " fields, but only ", inferred_size, " were detected");
      }
    }

    for (const auto& name_index : name_to_index_) {
      const auto& name = name_index.first;
      auto index = name_index.second;

      std::shared_ptr<ArrayData> reprs;
      RETURN_NOT_OK(repr_memos_[index]->GetArrayData(0, &reprs));

      if (reprs->length == 0) {
        return Status::Invalid("No non-null segments were available for field '", name,
                               "'; couldn't infer type");
      }

      std::shared_ptr<Field> current_field;
      std::shared_ptr<Array> dict;
      if (options_.schema) {
        // if we have a schema, use the schema type.
        current_field = options_.schema->field(index);
        auto cast_target = current_field->type();
        if (is_dictionary(cast_target->id())) {
          cast_target = checked_pointer_cast<DictionaryType>(cast_target)->value_type();
        }
        auto maybe_dict = compute::Cast(reprs, cast_target);
        if (!maybe_dict.ok()) {
          return Status::Invalid("Could not cast segments for partition field ",
                                 current_field->name(), " to requested type ",
                                 current_field->type()->ToString(),
                                 " because: ", maybe_dict.status());
        }
        dict = maybe_dict.ValueOrDie().make_array();
      } else {
        // try casting to int32, otherwise bail and just use the string reprs
        dict = compute::Cast(reprs, int32()).ValueOr(reprs).make_array();
        auto type = dict->type();
        if (options_.infer_dictionary) {
          // wrap the inferred type in dictionary()
          type = dictionary(int32(), std::move(type));
        }
        current_field = field(name, std::move(type));
      }
      fields[index] = std::move(current_field);
      dictionaries_[index] = std::move(dict);
    }

    Reset();
    return ::arrow::schema(std::move(fields));
  }

  std::vector<std::string> FieldNames() {
    std::vector<std::string> names(name_to_index_.size());

    for (auto kv : name_to_index_) {
      names[kv.second] = kv.first;
    }
    return names;
  }

  virtual void Reset() {
    name_to_index_.clear();
    repr_memos_.clear();
  }

  std::unique_ptr<internal::DictionaryMemoTable> MakeMemo() {
    return internal::make_unique<internal::DictionaryMemoTable>(default_memory_pool(),
                                                                utf8());
  }

  PartitioningFactoryOptions options_;
  ArrayVector dictionaries_;
  std::unordered_map<std::string, int> name_to_index_;
  std::vector<std::unique_ptr<internal::DictionaryMemoTable>> repr_memos_;
};

class DirectoryPartitioningFactory : public KeyValuePartitioningFactory {
 public:
  DirectoryPartitioningFactory(std::vector<std::string> field_names,
                               PartitioningFactoryOptions options)
      : KeyValuePartitioningFactory(options), field_names_(std::move(field_names)) {
    Reset();
  }

  std::string type_name() const override { return "schema"; }

  Result<std::shared_ptr<Schema>> Inspect(
      const std::vector<std::string>& paths) override {
    for (auto path : paths) {
      size_t field_index = 0;
      for (auto&& segment : fs::internal::SplitAbstractPath(path)) {
        if (field_index == field_names_.size()) break;

        RETURN_NOT_OK(InsertRepr(static_cast<int>(field_index++), segment));
      }
    }

    return DoInspect();
  }

  Result<std::shared_ptr<Partitioning>> Finish(
      const std::shared_ptr<Schema>& schema) const override {
    for (FieldRef ref : field_names_) {
      // ensure all of field_names_ are present in schema
      RETURN_NOT_OK(ref.FindOne(*schema).status());
    }

    // drop fields which aren't in field_names_
    auto out_schema = SchemaFromColumnNames(schema, field_names_);

    return std::make_shared<DirectoryPartitioning>(std::move(out_schema), dictionaries_);
  }

 private:
  void Reset() override {
    KeyValuePartitioningFactory::Reset();

    for (const auto& name : field_names_) {
      GetOrInsertField(name);
    }
  }

  std::vector<std::string> field_names_;
};

}  // namespace

std::shared_ptr<PartitioningFactory> DirectoryPartitioning::MakeFactory(
    std::vector<std::string> field_names, PartitioningFactoryOptions options) {
  return std::shared_ptr<PartitioningFactory>(
      new DirectoryPartitioningFactory(std::move(field_names), options));
}

util::optional<KeyValuePartitioning::Key> HivePartitioning::ParseKey(
    const std::string& segment, const std::string& null_fallback) {
  auto name_end = string_view(segment).find_first_of('=');
  // Not round-trippable
  if (name_end == string_view::npos) {
    return util::nullopt;
  }

  auto name = segment.substr(0, name_end);
  auto value = segment.substr(name_end + 1);
  if (value == null_fallback) {
    return Key{name, util::nullopt};
  }
  return Key{name, value};
}

std::vector<KeyValuePartitioning::Key> HivePartitioning::ParseKeys(
    const std::string& path) const {
  std::vector<Key> keys;

  for (const auto& segment : fs::internal::SplitAbstractPath(path)) {
    if (auto key = ParseKey(segment, null_fallback_)) {
      keys.push_back(std::move(*key));
    }
  }

  return keys;
}

Result<std::string> HivePartitioning::FormatValues(const ScalarVector& values) const {
  std::vector<std::string> segments(static_cast<size_t>(schema_->num_fields()));

  for (int i = 0; i < schema_->num_fields(); ++i) {
    const std::string& name = schema_->field(i)->name();

    if (values[i] == nullptr) {
      segments[i] = "";
    } else if (!values[i]->is_valid) {
      // If no key is available just provide a placeholder segment to maintain the
      // field_index <-> path nesting relation
      segments[i] = name + "=" + null_fallback_;
    } else {
      segments[i] = name + "=" + values[i]->ToString();
    }
  }

  return fs::internal::JoinAbstractPath(std::move(segments));
}

class HivePartitioningFactory : public KeyValuePartitioningFactory {
 public:
  explicit HivePartitioningFactory(HivePartitioningFactoryOptions options)
      : KeyValuePartitioningFactory(options), null_fallback_(options.null_fallback) {}

  std::string type_name() const override { return "hive"; }

  Result<std::shared_ptr<Schema>> Inspect(
      const std::vector<std::string>& paths) override {
    for (auto path : paths) {
      for (auto&& segment : fs::internal::SplitAbstractPath(path)) {
        if (auto key = HivePartitioning::ParseKey(segment, null_fallback_)) {
          RETURN_NOT_OK(InsertRepr(key->name, key->value));
        }
      }
    }

    field_names_ = FieldNames();
    return DoInspect();
  }

  Result<std::shared_ptr<Partitioning>> Finish(
      const std::shared_ptr<Schema>& schema) const override {
    if (dictionaries_.empty()) {
      return std::make_shared<HivePartitioning>(schema, dictionaries_);
    } else {
      for (FieldRef ref : field_names_) {
        // ensure all of field_names_ are present in schema
        RETURN_NOT_OK(ref.FindOne(*schema));
      }

      // drop fields which aren't in field_names_
      auto out_schema = SchemaFromColumnNames(schema, field_names_);

      return std::make_shared<HivePartitioning>(std::move(out_schema), dictionaries_,
                                                null_fallback_);
    }
  }

 private:
  const std::string null_fallback_;
  std::vector<std::string> field_names_;
};

std::shared_ptr<PartitioningFactory> HivePartitioning::MakeFactory(
    HivePartitioningFactoryOptions options) {
  return std::shared_ptr<PartitioningFactory>(new HivePartitioningFactory(options));
}

std::string StripPrefixAndFilename(const std::string& path, const std::string& prefix) {
  auto maybe_base_less = fs::internal::RemoveAncestor(prefix, path);
  auto base_less = maybe_base_less ? std::string(*maybe_base_less) : path;
  auto basename_filename = fs::internal::GetAbstractPathParent(base_less);
  return basename_filename.first;
}

std::vector<std::string> StripPrefixAndFilename(const std::vector<std::string>& paths,
                                                const std::string& prefix) {
  std::vector<std::string> result;
  result.reserve(paths.size());
  for (const auto& path : paths) {
    result.emplace_back(StripPrefixAndFilename(path, prefix));
  }
  return result;
}

std::vector<std::string> StripPrefixAndFilename(const std::vector<fs::FileInfo>& files,
                                                const std::string& prefix) {
  std::vector<std::string> result;
  result.reserve(files.size());
  for (const auto& info : files) {
    result.emplace_back(StripPrefixAndFilename(info.path(), prefix));
  }
  return result;
}

Result<std::shared_ptr<Schema>> PartitioningOrFactory::GetOrInferSchema(
    const std::vector<std::string>& paths) {
  if (auto part = partitioning()) {
    return part->schema();
  }

  return factory()->Inspect(paths);
}

}  // namespace dataset
}  // namespace arrow
