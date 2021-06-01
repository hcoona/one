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

#pragma once

#include <cmath>

#include "third_party/arrow/src/arrow/compute/api_aggregate.h"
#include "third_party/arrow/src/arrow/compute/kernels/aggregate_internal.h"
#include "third_party/arrow/src/arrow/compute/kernels/common.h"
#include "third_party/arrow/src/arrow/util/align_util.h"
#include "third_party/arrow/src/arrow/util/bit_block_counter.h"

namespace arrow {
namespace compute {
namespace aggregate {

void AddBasicAggKernels(KernelInit init,
                        const std::vector<std::shared_ptr<DataType>>& types,
                        std::shared_ptr<DataType> out_ty, ScalarAggregateFunction* func,
                        SimdLevel::type simd_level = SimdLevel::NONE);

void AddMinMaxKernels(KernelInit init,
                      const std::vector<std::shared_ptr<DataType>>& types,
                      ScalarAggregateFunction* func,
                      SimdLevel::type simd_level = SimdLevel::NONE);

// SIMD variants for kernels
void AddSumAvx2AggKernels(ScalarAggregateFunction* func);
void AddMeanAvx2AggKernels(ScalarAggregateFunction* func);
void AddMinMaxAvx2AggKernels(ScalarAggregateFunction* func);

void AddSumAvx512AggKernels(ScalarAggregateFunction* func);
void AddMeanAvx512AggKernels(ScalarAggregateFunction* func);
void AddMinMaxAvx512AggKernels(ScalarAggregateFunction* func);

// ----------------------------------------------------------------------
// Sum implementation

template <typename ArrowType, SimdLevel::type SimdLevel>
struct SumImpl : public ScalarAggregator {
  using ThisType = SumImpl<ArrowType, SimdLevel>;
  using CType = typename ArrowType::c_type;
  using SumType = typename FindAccumulatorType<ArrowType>::Type;
  using OutputType = typename TypeTraits<SumType>::ScalarType;

  void Consume(KernelContext*, const ExecBatch& batch) override {
    const auto& data = batch[0].array();
    this->count = data->length - data->GetNullCount();
    if (is_boolean_type<ArrowType>::value) {
      this->sum = static_cast<typename SumType::c_type>(BooleanArray(data).true_count());
    } else {
      this->sum =
          arrow::compute::detail::SumArray<CType, typename SumType::c_type>(*data);
    }
  }

  void MergeFrom(KernelContext*, KernelState&& src) override {
    const auto& other = checked_cast<const ThisType&>(src);
    this->count += other.count;
    this->sum += other.sum;
  }

  void Finalize(KernelContext*, Datum* out) override {
    if (this->count == 0) {
      out->value = std::make_shared<OutputType>();
    } else {
      out->value = MakeScalar(this->sum);
    }
  }

  size_t count = 0;
  typename SumType::c_type sum = 0;
};

template <typename ArrowType, SimdLevel::type SimdLevel>
struct MeanImpl : public SumImpl<ArrowType, SimdLevel> {
  void Finalize(KernelContext*, Datum* out) override {
    if (this->count == 0) {
      out->value = std::make_shared<DoubleScalar>();
    } else {
      const double mean = static_cast<double>(this->sum) / this->count;
      out->value = std::make_shared<DoubleScalar>(mean);
    }
  }
};

template <template <typename> class KernelClass>
struct SumLikeInit {
  std::unique_ptr<KernelState> state;
  KernelContext* ctx;
  const DataType& type;

  SumLikeInit(KernelContext* ctx, const DataType& type) : ctx(ctx), type(type) {}

  Status Visit(const DataType&) { return Status::NotImplemented("No sum implemented"); }

  Status Visit(const HalfFloatType&) {
    return Status::NotImplemented("No sum implemented");
  }

  Status Visit(const BooleanType&) {
    state.reset(new KernelClass<BooleanType>());
    return Status::OK();
  }

  template <typename Type>
  enable_if_number<Type, Status> Visit(const Type&) {
    state.reset(new KernelClass<Type>());
    return Status::OK();
  }

  std::unique_ptr<KernelState> Create() {
    ctx->SetStatus(VisitTypeInline(type, this));
    return std::move(state);
  }
};

// ----------------------------------------------------------------------
// MinMax implementation

template <typename ArrowType, SimdLevel::type SimdLevel, typename Enable = void>
struct MinMaxState {};

template <typename ArrowType, SimdLevel::type SimdLevel>
struct MinMaxState<ArrowType, SimdLevel, enable_if_boolean<ArrowType>> {
  using ThisType = MinMaxState<ArrowType, SimdLevel>;
  using T = typename ArrowType::c_type;

  ThisType& operator+=(const ThisType& rhs) {
    this->has_nulls |= rhs.has_nulls;
    this->has_values |= rhs.has_values;
    this->min = this->min && rhs.min;
    this->max = this->max || rhs.max;
    return *this;
  }

  void MergeOne(T value) {
    this->min = this->min && value;
    this->max = this->max || value;
  }

  T min = true;
  T max = false;
  bool has_nulls = false;
  bool has_values = false;
};

template <typename ArrowType, SimdLevel::type SimdLevel>
struct MinMaxState<ArrowType, SimdLevel, enable_if_integer<ArrowType>> {
  using ThisType = MinMaxState<ArrowType, SimdLevel>;
  using T = typename ArrowType::c_type;

  ThisType& operator+=(const ThisType& rhs) {
    this->has_nulls |= rhs.has_nulls;
    this->has_values |= rhs.has_values;
    this->min = std::min(this->min, rhs.min);
    this->max = std::max(this->max, rhs.max);
    return *this;
  }

  void MergeOne(T value) {
    this->min = std::min(this->min, value);
    this->max = std::max(this->max, value);
  }

  T min = std::numeric_limits<T>::max();
  T max = std::numeric_limits<T>::min();
  bool has_nulls = false;
  bool has_values = false;
};

template <typename ArrowType, SimdLevel::type SimdLevel>
struct MinMaxState<ArrowType, SimdLevel, enable_if_floating_point<ArrowType>> {
  using ThisType = MinMaxState<ArrowType, SimdLevel>;
  using T = typename ArrowType::c_type;

  ThisType& operator+=(const ThisType& rhs) {
    this->has_nulls |= rhs.has_nulls;
    this->has_values |= rhs.has_values;
    this->min = std::fmin(this->min, rhs.min);
    this->max = std::fmax(this->max, rhs.max);
    return *this;
  }

  void MergeOne(T value) {
    this->min = std::fmin(this->min, value);
    this->max = std::fmax(this->max, value);
  }

  T min = std::numeric_limits<T>::infinity();
  T max = -std::numeric_limits<T>::infinity();
  bool has_nulls = false;
  bool has_values = false;
};

template <typename ArrowType, SimdLevel::type SimdLevel>
struct MinMaxImpl : public ScalarAggregator {
  using ArrayType = typename TypeTraits<ArrowType>::ArrayType;
  using ThisType = MinMaxImpl<ArrowType, SimdLevel>;
  using StateType = MinMaxState<ArrowType, SimdLevel>;

  MinMaxImpl(const std::shared_ptr<DataType>& out_type, const MinMaxOptions& options)
      : out_type(out_type), options(options) {}

  void Consume(KernelContext*, const ExecBatch& batch) override {
    StateType local;

    ArrayType arr(batch[0].array());

    const auto null_count = arr.null_count();
    local.has_nulls = null_count > 0;
    local.has_values = (arr.length() - null_count) > 0;

    if (local.has_nulls && options.null_handling == MinMaxOptions::EMIT_NULL) {
      this->state = local;
      return;
    }

    if (local.has_nulls) {
      local += ConsumeWithNulls(arr);
    } else {  // All true values
      for (int64_t i = 0; i < arr.length(); i++) {
        local.MergeOne(arr.Value(i));
      }
    }
    this->state = local;
  }

  void MergeFrom(KernelContext*, KernelState&& src) override {
    const auto& other = checked_cast<const ThisType&>(src);
    this->state += other.state;
  }

  void Finalize(KernelContext*, Datum* out) override {
    using ScalarType = typename TypeTraits<ArrowType>::ScalarType;

    std::vector<std::shared_ptr<Scalar>> values;
    if (!state.has_values ||
        (state.has_nulls && options.null_handling == MinMaxOptions::EMIT_NULL)) {
      // (null, null)
      values = {std::make_shared<ScalarType>(), std::make_shared<ScalarType>()};
    } else {
      values = {std::make_shared<ScalarType>(state.min),
                std::make_shared<ScalarType>(state.max)};
    }
    out->value = std::make_shared<StructScalar>(std::move(values), this->out_type);
  }

  std::shared_ptr<DataType> out_type;
  MinMaxOptions options;
  MinMaxState<ArrowType, SimdLevel> state;

 private:
  StateType ConsumeWithNulls(const ArrayType& arr) const {
    StateType local;
    const int64_t length = arr.length();
    int64_t offset = arr.offset();
    const uint8_t* bitmap = arr.null_bitmap_data();
    int64_t idx = 0;

    const auto p = arrow::internal::BitmapWordAlign<1>(bitmap, offset, length);
    // First handle the leading bits
    const int64_t leading_bits = p.leading_bits;
    while (idx < leading_bits) {
      if (BitUtil::GetBit(bitmap, offset)) {
        local.MergeOne(arr.Value(idx));
      }
      idx++;
      offset++;
    }

    // The aligned parts scanned with BitBlockCounter
    arrow::internal::BitBlockCounter data_counter(bitmap, offset, length - leading_bits);
    auto current_block = data_counter.NextWord();
    while (idx < length) {
      if (current_block.AllSet()) {  // All true values
        int run_length = 0;
        // Scan forward until a block that has some false values (or the end)
        while (current_block.length > 0 && current_block.AllSet()) {
          run_length += current_block.length;
          current_block = data_counter.NextWord();
        }
        for (int64_t i = 0; i < run_length; i++) {
          local.MergeOne(arr.Value(idx + i));
        }
        idx += run_length;
        offset += run_length;
        // The current_block already computed, advance to next loop
        continue;
      } else if (!current_block.NoneSet()) {  // Some values are null
        BitmapReader reader(arr.null_bitmap_data(), offset, current_block.length);
        for (int64_t i = 0; i < current_block.length; i++) {
          if (reader.IsSet()) {
            local.MergeOne(arr.Value(idx + i));
          }
          reader.Next();
        }

        idx += current_block.length;
        offset += current_block.length;
      } else {  // All null values
        idx += current_block.length;
        offset += current_block.length;
      }
      current_block = data_counter.NextWord();
    }

    return local;
  }
};

template <SimdLevel::type SimdLevel>
struct BooleanMinMaxImpl : public MinMaxImpl<BooleanType, SimdLevel> {
  using StateType = MinMaxState<BooleanType, SimdLevel>;
  using ArrayType = typename TypeTraits<BooleanType>::ArrayType;
  using MinMaxImpl<BooleanType, SimdLevel>::MinMaxImpl;
  using MinMaxImpl<BooleanType, SimdLevel>::options;

  void Consume(KernelContext*, const ExecBatch& batch) override {
    StateType local;
    ArrayType arr(batch[0].array());

    const auto arr_length = arr.length();
    const auto null_count = arr.null_count();
    const auto valid_count = arr_length - null_count;

    local.has_nulls = null_count > 0;
    local.has_values = valid_count > 0;
    if (local.has_nulls && options.null_handling == MinMaxOptions::EMIT_NULL) {
      this->state = local;
      return;
    }

    const auto true_count = arr.true_count();
    const auto false_count = valid_count - true_count;
    local.max = true_count > 0;
    local.min = false_count == 0;

    this->state = local;
  }
};

template <SimdLevel::type SimdLevel>
struct MinMaxInitState {
  std::unique_ptr<KernelState> state;
  KernelContext* ctx;
  const DataType& in_type;
  const std::shared_ptr<DataType>& out_type;
  const MinMaxOptions& options;

  MinMaxInitState(KernelContext* ctx, const DataType& in_type,
                  const std::shared_ptr<DataType>& out_type, const MinMaxOptions& options)
      : ctx(ctx), in_type(in_type), out_type(out_type), options(options) {}

  Status Visit(const DataType&) {
    return Status::NotImplemented("No min/max implemented");
  }

  Status Visit(const HalfFloatType&) {
    return Status::NotImplemented("No min/max implemented");
  }

  Status Visit(const BooleanType&) {
    state.reset(new BooleanMinMaxImpl<SimdLevel>(out_type, options));
    return Status::OK();
  }

  template <typename Type>
  enable_if_number<Type, Status> Visit(const Type&) {
    state.reset(new MinMaxImpl<Type, SimdLevel>(out_type, options));
    return Status::OK();
  }

  std::unique_ptr<KernelState> Create() {
    ctx->SetStatus(VisitTypeInline(in_type, this));
    return std::move(state);
  }
};

}  // namespace aggregate
}  // namespace compute
}  // namespace arrow
