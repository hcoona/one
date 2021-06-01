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

#include "third_party/arrow/src/arrow/array/array_base.h"
#include "third_party/arrow/src/arrow/array/builder_primitive.h"
#include "third_party/arrow/src/arrow/compute/api_scalar.h"
#include "third_party/arrow/src/arrow/compute/cast.h"
#include "third_party/arrow/src/arrow/compute/kernels/common.h"
#include "third_party/arrow/src/arrow/compute/kernels/util_internal.h"
#include "third_party/arrow/src/arrow/util/bit_util.h"
#include "third_party/arrow/src/arrow/util/bitmap_writer.h"
#include "third_party/arrow/src/arrow/util/hashing.h"
#include "third_party/arrow/src/arrow/visitor_inline.h"

namespace arrow {

using internal::checked_cast;
using internal::HashTraits;

namespace compute {
namespace internal {
namespace {

template <typename Type>
struct SetLookupState : public KernelState {
  explicit SetLookupState(MemoryPool* pool) : lookup_table(pool, 0) {}

  Status Init(const SetLookupOptions& options) {
    if (options.value_set.kind() == Datum::ARRAY) {
      RETURN_NOT_OK(AddArrayValueSet(*options.value_set.array()));
    } else if (options.value_set.kind() == Datum::CHUNKED_ARRAY) {
      const ChunkedArray& value_set = *options.value_set.chunked_array();
      for (const std::shared_ptr<Array>& chunk : value_set.chunks()) {
        RETURN_NOT_OK(AddArrayValueSet(*chunk->data()));
      }
    } else {
      return Status::Invalid("value_set should be an array or chunked array");
    }
    if (lookup_table.size() != options.value_set.length()) {
      return Status::NotImplemented("duplicate values in value_set");
    }
    if (!options.skip_nulls) {
      null_index = lookup_table.GetNull();
    }
    return Status::OK();
  }

  Status AddArrayValueSet(const ArrayData& data) {
    using T = typename GetViewType<Type>::T;
    auto visit_valid = [&](T v) {
      int32_t unused_memo_index;
      return lookup_table.GetOrInsert(v, &unused_memo_index);
    };
    auto visit_null = [&]() {
      lookup_table.GetOrInsertNull();
      return Status::OK();
    };

    return VisitArrayDataInline<Type>(data, visit_valid, visit_null);
  }

  using MemoTable = typename HashTraits<Type>::MemoTableType;
  MemoTable lookup_table;
  int32_t null_index = -1;
};

template <>
struct SetLookupState<NullType> : public KernelState {
  explicit SetLookupState(MemoryPool*) {}

  Status Init(const SetLookupOptions& options) {
    value_set_has_null = (options.value_set.length() > 0) && !options.skip_nulls;
    return Status::OK();
  }

  bool value_set_has_null;
};

// TODO: Put this concept somewhere reusable
template <int width>
struct UnsignedIntType;

template <>
struct UnsignedIntType<1> {
  using Type = UInt8Type;
};

template <>
struct UnsignedIntType<2> {
  using Type = UInt16Type;
};

template <>
struct UnsignedIntType<4> {
  using Type = UInt32Type;
};

template <>
struct UnsignedIntType<8> {
  using Type = UInt64Type;
};

// Constructing the type requires a type parameter
struct InitStateVisitor {
  KernelContext* ctx;
  SetLookupOptions options;
  const std::shared_ptr<DataType>& arg_type;
  std::unique_ptr<KernelState> result;

  InitStateVisitor(KernelContext* ctx, const KernelInitArgs& args)
      : ctx(ctx),
        options(*checked_cast<const SetLookupOptions*>(args.options)),
        arg_type(args.inputs[0].type) {}

  template <typename Type>
  Status Init() {
    using StateType = SetLookupState<Type>;
    result.reset(new StateType(ctx->exec_context()->memory_pool()));
    return static_cast<StateType*>(result.get())->Init(options);
  }

  Status Visit(const DataType&) { return Init<NullType>(); }

  template <typename Type>
  enable_if_boolean<Type, Status> Visit(const Type&) {
    return Init<BooleanType>();
  }

  template <typename Type>
  enable_if_t<has_c_type<Type>::value && !is_boolean_type<Type>::value, Status> Visit(
      const Type&) {
    return Init<typename UnsignedIntType<sizeof(typename Type::c_type)>::Type>();
  }

  template <typename Type>
  enable_if_base_binary<Type, Status> Visit(const Type&) {
    return Init<typename Type::PhysicalType>();
  }

  // Handle Decimal128Type, FixedSizeBinaryType
  Status Visit(const FixedSizeBinaryType& type) { return Init<FixedSizeBinaryType>(); }

  Status GetResult(std::unique_ptr<KernelState>* out) {
    if (!options.value_set.type()->Equals(arg_type)) {
      ARROW_ASSIGN_OR_RAISE(
          options.value_set,
          Cast(options.value_set, CastOptions::Safe(arg_type), ctx->exec_context()));
    }

    RETURN_NOT_OK(VisitTypeInline(*arg_type, this));
    *out = std::move(result);
    return Status::OK();
  }
};

std::unique_ptr<KernelState> InitSetLookup(KernelContext* ctx,
                                           const KernelInitArgs& args) {
  if (args.options == nullptr) {
    ctx->SetStatus(Status::Invalid(
        "Attempted to call a set lookup function without SetLookupOptions"));
    return nullptr;
  }

  std::unique_ptr<KernelState> result;
  ctx->SetStatus(InitStateVisitor{ctx, args}.GetResult(&result));
  return result;
}

struct IndexInVisitor {
  KernelContext* ctx;
  const ArrayData& data;
  Datum* out;
  Int32Builder builder;

  IndexInVisitor(KernelContext* ctx, const ArrayData& data, Datum* out)
      : ctx(ctx), data(data), out(out), builder(ctx->exec_context()->memory_pool()) {}

  Status Visit(const DataType& type) {
    DCHECK_EQ(type.id(), Type::NA);
    const auto& state = checked_cast<const SetLookupState<NullType>&>(*ctx->state());
    if (data.length != 0) {
      // skip_nulls is honored for consistency with other types
      if (state.value_set_has_null) {
        RETURN_NOT_OK(this->builder.Reserve(data.length));
        for (int64_t i = 0; i < data.length; ++i) {
          this->builder.UnsafeAppend(0);
        }
      } else {
        RETURN_NOT_OK(this->builder.AppendNulls(data.length));
      }
    }
    return Status::OK();
  }

  template <typename Type>
  Status ProcessIndexIn() {
    using T = typename GetViewType<Type>::T;

    const auto& state = checked_cast<const SetLookupState<Type>&>(*ctx->state());

    RETURN_NOT_OK(this->builder.Reserve(data.length));
    VisitArrayDataInline<Type>(
        data,
        [&](T v) {
          int32_t index = state.lookup_table.Get(v);
          if (index != -1) {
            // matching needle; output index from value_set
            this->builder.UnsafeAppend(index);
          } else {
            // no matching needle; output null
            this->builder.UnsafeAppendNull();
          }
        },
        [&]() {
          if (state.null_index != -1) {
            // value_set included null
            this->builder.UnsafeAppend(state.null_index);
          } else {
            // value_set does not include null; output null
            this->builder.UnsafeAppendNull();
          }
        });
    return Status::OK();
  }

  template <typename Type>
  enable_if_boolean<Type, Status> Visit(const Type&) {
    return ProcessIndexIn<BooleanType>();
  }

  template <typename Type>
  enable_if_t<has_c_type<Type>::value && !is_boolean_type<Type>::value, Status> Visit(
      const Type&) {
    return ProcessIndexIn<
        typename UnsignedIntType<sizeof(typename Type::c_type)>::Type>();
  }

  template <typename Type>
  enable_if_base_binary<Type, Status> Visit(const Type&) {
    return ProcessIndexIn<typename Type::PhysicalType>();
  }

  // Handle Decimal128Type, FixedSizeBinaryType
  Status Visit(const FixedSizeBinaryType& type) {
    return ProcessIndexIn<FixedSizeBinaryType>();
  }

  Status Execute() {
    Status s = VisitTypeInline(*data.type, this);
    if (!s.ok()) {
      return s;
    }
    std::shared_ptr<ArrayData> out_data;
    RETURN_NOT_OK(this->builder.FinishInternal(&out_data));
    out->value = std::move(out_data);
    return Status::OK();
  }
};

void ExecIndexIn(KernelContext* ctx, const ExecBatch& batch, Datum* out) {
  KERNEL_RETURN_IF_ERROR(ctx, IndexInVisitor(ctx, *batch[0].array(), out).Execute());
}

// ----------------------------------------------------------------------

// IsIn writes the results into a preallocated boolean data bitmap
struct IsInVisitor {
  KernelContext* ctx;
  const ArrayData& data;
  Datum* out;

  IsInVisitor(KernelContext* ctx, const ArrayData& data, Datum* out)
      : ctx(ctx), data(data), out(out) {}

  Status Visit(const DataType& type) {
    DCHECK_EQ(type.id(), Type::NA);
    const auto& state = checked_cast<const SetLookupState<NullType>&>(*ctx->state());
    ArrayData* output = out->mutable_array();
    // skip_nulls is honored for consistency with other types
    BitUtil::SetBitsTo(output->buffers[1]->mutable_data(), output->offset, output->length,
                       state.value_set_has_null);
    return Status::OK();
  }

  template <typename Type>
  Status ProcessIsIn() {
    using T = typename GetViewType<Type>::T;
    const auto& state = checked_cast<const SetLookupState<Type>&>(*ctx->state());
    ArrayData* output = out->mutable_array();

    FirstTimeBitmapWriter writer(output->buffers[1]->mutable_data(), output->offset,
                                 output->length);

    VisitArrayDataInline<Type>(
        this->data,
        [&](T v) {
          if (state.lookup_table.Get(v) != -1) {
            writer.Set();
          } else {
            writer.Clear();
          }
          writer.Next();
        },
        [&]() {
          if (state.null_index != -1) {
            writer.Set();
          } else {
            writer.Clear();
          }
          writer.Next();
        });
    writer.Finish();
    return Status::OK();
  }

  template <typename Type>
  enable_if_boolean<Type, Status> Visit(const Type&) {
    return ProcessIsIn<BooleanType>();
  }

  template <typename Type>
  enable_if_t<has_c_type<Type>::value && !is_boolean_type<Type>::value, Status> Visit(
      const Type&) {
    return ProcessIsIn<typename UnsignedIntType<sizeof(typename Type::c_type)>::Type>();
  }

  template <typename Type>
  enable_if_base_binary<Type, Status> Visit(const Type&) {
    return ProcessIsIn<typename Type::PhysicalType>();
  }

  // Handle Decimal128Type, FixedSizeBinaryType
  Status Visit(const FixedSizeBinaryType& type) {
    return ProcessIsIn<FixedSizeBinaryType>();
  }

  Status Execute() { return VisitTypeInline(*data.type, this); }
};

void ExecIsIn(KernelContext* ctx, const ExecBatch& batch, Datum* out) {
  KERNEL_RETURN_IF_ERROR(ctx, IsInVisitor(ctx, *batch[0].array(), out).Execute());
}

// Unary set lookup kernels available for the following input types
//
// * Null type
// * Boolean
// * Numeric
// * Simple temporal types (date, time, timestamp)
// * Base binary types
// * Decimal

void AddBasicSetLookupKernels(ScalarKernel kernel,
                              const std::shared_ptr<DataType>& out_ty,
                              ScalarFunction* func) {
  auto AddKernels = [&](const std::vector<std::shared_ptr<DataType>>& types) {
    for (const std::shared_ptr<DataType>& ty : types) {
      kernel.signature = KernelSignature::Make({ty}, out_ty);
      DCHECK_OK(func->AddKernel(kernel));
    }
  };

  AddKernels(BaseBinaryTypes());
  AddKernels(NumericTypes());
  AddKernels(TemporalTypes());

  std::vector<Type::type> other_types = {Type::BOOL, Type::DECIMAL,
                                         Type::FIXED_SIZE_BINARY};
  for (auto ty : other_types) {
    kernel.signature = KernelSignature::Make({InputType::Array(ty)}, out_ty);
    DCHECK_OK(func->AddKernel(kernel));
  }
}

// Enables calling is_in with CallFunction as though it were binary.
class IsInMetaBinary : public MetaFunction {
 public:
  IsInMetaBinary()
      : MetaFunction("is_in_meta_binary", Arity::Binary(), /*doc=*/nullptr) {}

  Result<Datum> ExecuteImpl(const std::vector<Datum>& args,
                            const FunctionOptions* options,
                            ExecContext* ctx) const override {
    if (options != nullptr) {
      return Status::Invalid("Unexpected options for 'is_in_meta_binary' function");
    }
    return IsIn(args[0], args[1], ctx);
  }
};

// Enables calling index_in with CallFunction as though it were binary.
class IndexInMetaBinary : public MetaFunction {
 public:
  IndexInMetaBinary()
      : MetaFunction("index_in_meta_binary", Arity::Binary(), /*doc=*/nullptr) {}

  Result<Datum> ExecuteImpl(const std::vector<Datum>& args,
                            const FunctionOptions* options,
                            ExecContext* ctx) const override {
    if (options != nullptr) {
      return Status::Invalid("Unexpected options for 'index_in_meta_binary' function");
    }
    return IndexIn(args[0], args[1], ctx);
  }
};

struct SetLookupFunction : ScalarFunction {
  using ScalarFunction::ScalarFunction;

  Result<const Kernel*> DispatchBest(std::vector<ValueDescr>* values) const override {
    EnsureDictionaryDecoded(values);
    return DispatchExact(*values);
  }
};

const FunctionDoc is_in_doc{
    "Find each element in a set of values",
    ("For each element in `values`, return true if it is found in a given\n"
     "set of values, false otherwise.\n"
     "The set of values to look for must be given in SetLookupOptions.\n"
     "By default, nulls are matched against the value set, this can be\n"
     "changed in SetLookupOptions."),
    {"values"},
    "SetLookupOptions"};

const FunctionDoc index_in_doc{
    "Return index of each element in a set of values",
    ("For each element in `values`, return its index in a given set of\n"
     "values, or null if it is not found there.\n"
     "The set of values to look for must be given in SetLookupOptions.\n"
     "By default, nulls are matched against the value set, this can be\n"
     "changed in SetLookupOptions."),
    {"values"},
    "SetLookupOptions"};

}  // namespace

void RegisterScalarSetLookup(FunctionRegistry* registry) {
  // IsIn writes its boolean output into preallocated memory
  {
    ScalarKernel isin_base;
    isin_base.init = InitSetLookup;
    isin_base.exec =
        TrivialScalarUnaryAsArraysExec(ExecIsIn, NullHandling::OUTPUT_NOT_NULL);
    isin_base.null_handling = NullHandling::OUTPUT_NOT_NULL;
    auto is_in = std::make_shared<SetLookupFunction>("is_in", Arity::Unary(), &is_in_doc);

    AddBasicSetLookupKernels(isin_base, /*output_type=*/boolean(), is_in.get());

    isin_base.signature = KernelSignature::Make({null()}, boolean());
    DCHECK_OK(is_in->AddKernel(isin_base));
    DCHECK_OK(registry->AddFunction(is_in));

    DCHECK_OK(registry->AddFunction(std::make_shared<IsInMetaBinary>()));
  }

  // IndexIn uses Int32Builder and so is responsible for all its own allocation
  {
    ScalarKernel index_in_base;
    index_in_base.init = InitSetLookup;
    index_in_base.exec = TrivialScalarUnaryAsArraysExec(
        ExecIndexIn, NullHandling::COMPUTED_NO_PREALLOCATE);
    index_in_base.null_handling = NullHandling::COMPUTED_NO_PREALLOCATE;
    index_in_base.mem_allocation = MemAllocation::NO_PREALLOCATE;
    auto index_in =
        std::make_shared<SetLookupFunction>("index_in", Arity::Unary(), &index_in_doc);

    AddBasicSetLookupKernels(index_in_base, /*output_type=*/int32(), index_in.get());

    index_in_base.signature = KernelSignature::Make({null()}, int32());
    DCHECK_OK(index_in->AddKernel(index_in_base));
    DCHECK_OK(registry->AddFunction(index_in));

    DCHECK_OK(registry->AddFunction(std::make_shared<IndexInMetaBinary>()));
  }
}

}  // namespace internal
}  // namespace compute
}  // namespace arrow
