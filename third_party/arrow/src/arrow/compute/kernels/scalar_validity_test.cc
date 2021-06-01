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

#include "third_party/googletest/googletest/include/gtest/gtest.h"

#include "third_party/arrow/src/arrow/array.h"
#include "third_party/arrow/src/arrow/compute/api.h"
#include "third_party/arrow/src/arrow/compute/kernels/test_util.h"
#include "third_party/arrow/src/arrow/testing/gtest_common.h"
#include "third_party/arrow/src/arrow/testing/gtest_util.h"
#include "third_party/arrow/src/arrow/testing/random.h"
#include "third_party/arrow/src/arrow/type.h"
#include "third_party/arrow/src/arrow/type_traits.h"
#include "third_party/arrow/src/arrow/util/bitmap_reader.h"
#include "third_party/arrow/src/arrow/util/checked_cast.h"

namespace arrow {
namespace compute {

template <typename ArrowType>
class TestValidityKernels : public ::testing::Test {
 protected:
  static std::shared_ptr<DataType> type_singleton() {
    return TypeTraits<ArrowType>::type_singleton();
  }
};

using TestBooleanValidityKernels = TestValidityKernels<BooleanType>;
using TestFloatValidityKernels = TestValidityKernels<FloatType>;
using TestDoubleValidityKernels = TestValidityKernels<DoubleType>;

TEST_F(TestBooleanValidityKernels, ArrayIsValid) {
  CheckScalarUnary("is_valid", type_singleton(), "[]", type_singleton(), "[]");
  CheckScalarUnary("is_valid", type_singleton(), "[null]", type_singleton(), "[false]");
  CheckScalarUnary("is_valid", type_singleton(), "[1]", type_singleton(), "[true]");
  CheckScalarUnary("is_valid", type_singleton(), "[null, 1, 0, null]", type_singleton(),
                   "[false, true, true, false]");
}

TEST_F(TestBooleanValidityKernels, IsValidIsNullNullType) {
  CheckScalarUnary("is_null", std::make_shared<NullArray>(5),
                   ArrayFromJSON(boolean(), "[true, true, true, true, true]"));
  CheckScalarUnary("is_valid", std::make_shared<NullArray>(5),
                   ArrayFromJSON(boolean(), "[false, false, false, false, false]"));
}

TEST_F(TestBooleanValidityKernels, ArrayIsValidBufferPassthruOptimization) {
  Datum arg = ArrayFromJSON(boolean(), "[null, 1, 0, null]");
  ASSERT_OK_AND_ASSIGN(auto validity, arrow::compute::IsValid(arg));
  ASSERT_EQ(validity.array()->buffers[1], arg.array()->buffers[0]);
}

TEST_F(TestBooleanValidityKernels, ScalarIsValid) {
  CheckScalarUnary("is_valid", MakeScalar(19.7), MakeScalar(true));
  CheckScalarUnary("is_valid", MakeNullScalar(float64()), MakeScalar(false));
}

TEST_F(TestBooleanValidityKernels, ArrayIsNull) {
  CheckScalarUnary("is_null", type_singleton(), "[]", type_singleton(), "[]");
  CheckScalarUnary("is_null", type_singleton(), "[null]", type_singleton(), "[true]");
  CheckScalarUnary("is_null", type_singleton(), "[1]", type_singleton(), "[false]");
  CheckScalarUnary("is_null", type_singleton(), "[null, 1, 0, null]", type_singleton(),
                   "[true, false, false, true]");
}

TEST_F(TestBooleanValidityKernels, IsNullSetsZeroNullCount) {
  auto arr = ArrayFromJSON(int32(), "[1, 2, 3, 4]");
  std::shared_ptr<ArrayData> result = (*IsNull(arr)).array();
  ASSERT_EQ(result->null_count, 0);
}

TEST_F(TestBooleanValidityKernels, ScalarIsNull) {
  CheckScalarUnary("is_null", MakeScalar(19.7), MakeScalar(false));
  CheckScalarUnary("is_null", MakeNullScalar(float64()), MakeScalar(true));
}

TEST_F(TestFloatValidityKernels, FloatArrayIsNan) {
  // All NaN
  CheckScalarUnary("is_nan", ArrayFromJSON(float32(), "[NaN, NaN, NaN, NaN, NaN]"),
                   ArrayFromJSON(boolean(), "[true, true, true, true, true]"));
  // No NaN
  CheckScalarUnary("is_nan", ArrayFromJSON(float32(), "[0.0, 1.0, 2.0, 3.0, 4.0, null]"),
                   ArrayFromJSON(boolean(), "[false, false, false, false, false, null]"));
  // Some NaNs
  CheckScalarUnary("is_nan", ArrayFromJSON(float32(), "[0.0, NaN, 2.0, NaN, 4.0, null]"),
                   ArrayFromJSON(boolean(), "[false, true, false, true, false, null]"));
}

TEST_F(TestDoubleValidityKernels, DoubleArrayIsNan) {
  // All NaN
  CheckScalarUnary("is_nan", ArrayFromJSON(float64(), "[NaN, NaN, NaN, NaN, NaN]"),
                   ArrayFromJSON(boolean(), "[true, true, true, true, true]"));
  // No NaN
  CheckScalarUnary("is_nan", ArrayFromJSON(float64(), "[0.0, 1.0, 2.0, 3.0, 4.0, null]"),
                   ArrayFromJSON(boolean(), "[false, false, false, false, false, null]"));
  // Some NaNs
  CheckScalarUnary("is_nan", ArrayFromJSON(float64(), "[0.0, NaN, 2.0, NaN, 4.0, null]"),
                   ArrayFromJSON(boolean(), "[false, true, false, true, false, null]"));
}

TEST_F(TestFloatValidityKernels, FloatScalarIsNan) {
  CheckScalarUnary("is_nan", MakeNullScalar(float32()), MakeNullScalar(boolean()));
  CheckScalarUnary("is_nan", MakeScalar(42.0f), MakeScalar(false));
  CheckScalarUnary("is_nan", MakeScalar(std::nanf("")), MakeScalar(true));
}

TEST_F(TestDoubleValidityKernels, DoubleScalarIsNan) {
  CheckScalarUnary("is_nan", MakeNullScalar(float64()), MakeNullScalar(boolean()));
  CheckScalarUnary("is_nan", MakeScalar(42.0), MakeScalar(false));
  CheckScalarUnary("is_nan", MakeScalar(std::nan("")), MakeScalar(true));
}

}  // namespace compute
}  // namespace arrow
