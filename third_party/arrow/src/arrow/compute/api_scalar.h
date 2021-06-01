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

// Eager evaluation convenience APIs for invoking common functions, including
// necessary memory allocations

#pragma once

#include <string>
#include <utility>

#include "third_party/arrow/src/arrow/compute/exec.h"  // IWYU pragma: keep
#include "third_party/arrow/src/arrow/compute/function.h"
#include "third_party/arrow/src/arrow/datum.h"
#include "third_party/arrow/src/arrow/result.h"
#include "third_party/arrow/src/arrow/util/macros.h"
#include "third_party/arrow/src/arrow/util/visibility.h"

namespace arrow {
namespace compute {

/// \addtogroup compute-concrete-options
///
/// @{

struct ArithmeticOptions : public FunctionOptions {
  ArithmeticOptions() : check_overflow(false) {}
  bool check_overflow;
};

struct ARROW_EXPORT MatchSubstringOptions : public FunctionOptions {
  explicit MatchSubstringOptions(std::string pattern) : pattern(std::move(pattern)) {}

  /// The exact substring (or regex, depending on kernel) to look for inside input values.
  std::string pattern;
};

struct ARROW_EXPORT SplitOptions : public FunctionOptions {
  explicit SplitOptions(int64_t max_splits = -1, bool reverse = false)
      : max_splits(max_splits), reverse(reverse) {}

  /// Maximum number of splits allowed, or unlimited when -1
  int64_t max_splits;
  /// Start splitting from the end of the string (only relevant when max_splits != -1)
  bool reverse;
};

struct ARROW_EXPORT SplitPatternOptions : public SplitOptions {
  explicit SplitPatternOptions(std::string pattern, int64_t max_splits = -1,
                               bool reverse = false)
      : SplitOptions(max_splits, reverse), pattern(std::move(pattern)) {}

  /// The exact substring to look for inside input values.
  std::string pattern;
};

struct ARROW_EXPORT ReplaceSubstringOptions : public FunctionOptions {
  explicit ReplaceSubstringOptions(std::string pattern, std::string replacement,
                                   int64_t max_replacements = -1)
      : pattern(std::move(pattern)),
        replacement(std::move(replacement)),
        max_replacements(max_replacements) {}

  /// Pattern to match, literal, or regular expression depending on which kernel is used
  std::string pattern;
  /// String to replace the pattern with
  std::string replacement;
  /// Max number of substrings to replace (-1 means unbounded)
  int64_t max_replacements;
};

struct ARROW_EXPORT ExtractRegexOptions : public FunctionOptions {
  explicit ExtractRegexOptions(std::string pattern) : pattern(std::move(pattern)) {}

  /// Regular expression with named capture fields
  std::string pattern;
};

/// Options for IsIn and IndexIn functions
struct ARROW_EXPORT SetLookupOptions : public FunctionOptions {
  explicit SetLookupOptions(Datum value_set, bool skip_nulls = false)
      : value_set(std::move(value_set)), skip_nulls(skip_nulls) {}

  /// The set of values to look up input values into.
  Datum value_set;
  /// Whether nulls in `value_set` count for lookup.
  ///
  /// If true, any null in `value_set` is ignored and nulls in the input
  /// produce null (IndexIn) or false (IsIn) values in the output.
  /// If false, any null in `value_set` is successfully matched in
  /// the input.
  bool skip_nulls;
};

struct ARROW_EXPORT StrptimeOptions : public FunctionOptions {
  explicit StrptimeOptions(std::string format, TimeUnit::type unit)
      : format(std::move(format)), unit(unit) {}

  std::string format;
  TimeUnit::type unit;
};

struct ARROW_EXPORT TrimOptions : public FunctionOptions {
  explicit TrimOptions(std::string characters) : characters(std::move(characters)) {}

  /// The individual characters that can be trimmed from the string.
  std::string characters;
};

enum CompareOperator : int8_t {
  EQUAL,
  NOT_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
};

struct CompareOptions : public FunctionOptions {
  explicit CompareOptions(CompareOperator op) : op(op) {}

  enum CompareOperator op;
};

struct ARROW_EXPORT ProjectOptions : public FunctionOptions {
  ProjectOptions(std::vector<std::string> n, std::vector<bool> r,
                 std::vector<std::shared_ptr<const KeyValueMetadata>> m)
      : field_names(std::move(n)),
        field_nullability(std::move(r)),
        field_metadata(std::move(m)) {}

  explicit ProjectOptions(std::vector<std::string> n)
      : field_names(std::move(n)),
        field_nullability(field_names.size(), true),
        field_metadata(field_names.size(), NULLPTR) {}

  /// Names for wrapped columns
  std::vector<std::string> field_names;

  /// Nullability bits for wrapped columns
  std::vector<bool> field_nullability;

  /// Metadata attached to wrapped columns
  std::vector<std::shared_ptr<const KeyValueMetadata>> field_metadata;
};

/// @}

/// \brief Add two values together. Array values must be the same length. If
/// either addend is null the result will be null.
///
/// \param[in] left the first addend
/// \param[in] right the second addend
/// \param[in] options arithmetic options (overflow handling), optional
/// \param[in] ctx the function execution context, optional
/// \return the elementwise sum
ARROW_EXPORT
Result<Datum> Add(const Datum& left, const Datum& right,
                  ArithmeticOptions options = ArithmeticOptions(),
                  ExecContext* ctx = NULLPTR);

/// \brief Subtract two values. Array values must be the same length. If the
/// minuend or subtrahend is null the result will be null.
///
/// \param[in] left the value subtracted from (minuend)
/// \param[in] right the value by which the minuend is reduced (subtrahend)
/// \param[in] options arithmetic options (overflow handling), optional
/// \param[in] ctx the function execution context, optional
/// \return the elementwise difference
ARROW_EXPORT
Result<Datum> Subtract(const Datum& left, const Datum& right,
                       ArithmeticOptions options = ArithmeticOptions(),
                       ExecContext* ctx = NULLPTR);

/// \brief Multiply two values. Array values must be the same length. If either
/// factor is null the result will be null.
///
/// \param[in] left the first factor
/// \param[in] right the second factor
/// \param[in] options arithmetic options (overflow handling), optional
/// \param[in] ctx the function execution context, optional
/// \return the elementwise product
ARROW_EXPORT
Result<Datum> Multiply(const Datum& left, const Datum& right,
                       ArithmeticOptions options = ArithmeticOptions(),
                       ExecContext* ctx = NULLPTR);

/// \brief Divide two values. Array values must be the same length. If either
/// argument is null the result will be null. For integer types, if there is
/// a zero divisor, an error will be raised.
///
/// \param[in] left the dividend
/// \param[in] right the divisor
/// \param[in] options arithmetic options (enable/disable overflow checking), optional
/// \param[in] ctx the function execution context, optional
/// \return the elementwise quotient
ARROW_EXPORT
Result<Datum> Divide(const Datum& left, const Datum& right,
                     ArithmeticOptions options = ArithmeticOptions(),
                     ExecContext* ctx = NULLPTR);

/// \brief Raise the values of base array to the power of the exponent array values.
/// Array values must be the same length. If either base or exponent is null the result
/// will be null.
///
/// \param[in] left the base
/// \param[in] right the exponent
/// \param[in] options arithmetic options (enable/disable overflow checking), optional
/// \param[in] ctx the function execution context, optional
/// \return the elementwise base value raised to the power of exponent
ARROW_EXPORT
Result<Datum> Power(const Datum& left, const Datum& right,
                    ArithmeticOptions options = ArithmeticOptions(),
                    ExecContext* ctx = NULLPTR);

/// \brief Compare a numeric array with a scalar.
///
/// \param[in] left datum to compare, must be an Array
/// \param[in] right datum to compare, must be a Scalar of the same type than
///            left Datum.
/// \param[in] options compare options
/// \param[in] ctx the function execution context, optional
/// \return resulting datum
///
/// Note on floating point arrays, this uses ieee-754 compare semantics.
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> Compare(const Datum& left, const Datum& right,
                      struct CompareOptions options, ExecContext* ctx = NULLPTR);

/// \brief Invert the values of a boolean datum
/// \param[in] value datum to invert
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> Invert(const Datum& value, ExecContext* ctx = NULLPTR);

/// \brief Element-wise AND of two boolean datums which always propagates nulls
/// (null and false is null).
///
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> And(const Datum& left, const Datum& right, ExecContext* ctx = NULLPTR);

/// \brief Element-wise AND of two boolean datums with a Kleene truth table
/// (null and false is false).
///
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> KleeneAnd(const Datum& left, const Datum& right,
                        ExecContext* ctx = NULLPTR);

/// \brief Element-wise OR of two boolean datums which always propagates nulls
/// (null and true is null).
///
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> Or(const Datum& left, const Datum& right, ExecContext* ctx = NULLPTR);

/// \brief Element-wise OR of two boolean datums with a Kleene truth table
/// (null or true is true).
///
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> KleeneOr(const Datum& left, const Datum& right, ExecContext* ctx = NULLPTR);

/// \brief Element-wise XOR of two boolean datums
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> Xor(const Datum& left, const Datum& right, ExecContext* ctx = NULLPTR);

/// \brief Element-wise AND NOT of two boolean datums which always propagates nulls
/// (null and not true is null).
///
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 3.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> AndNot(const Datum& left, const Datum& right, ExecContext* ctx = NULLPTR);

/// \brief Element-wise AND NOT of two boolean datums with a Kleene truth table
/// (false and not null is false, null and not true is false).
///
/// \param[in] left left operand
/// \param[in] right right operand
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 3.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> KleeneAndNot(const Datum& left, const Datum& right,
                           ExecContext* ctx = NULLPTR);

/// \brief IsIn returns true for each element of `values` that is contained in
/// `value_set`
///
/// Behaviour of nulls is governed by SetLookupOptions::skip_nulls.
///
/// \param[in] values array-like input to look up in value_set
/// \param[in] options SetLookupOptions
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> IsIn(const Datum& values, const SetLookupOptions& options,
                   ExecContext* ctx = NULLPTR);
ARROW_EXPORT
Result<Datum> IsIn(const Datum& values, const Datum& value_set,
                   ExecContext* ctx = NULLPTR);

/// \brief IndexIn examines each slot in the values against a value_set array.
/// If the value is not found in value_set, null will be output.
/// If found, the index of occurrence within value_set (ignoring duplicates)
/// will be output.
///
/// For example given values = [99, 42, 3, null] and
/// value_set = [3, 3, 99], the output will be = [1, null, 0, null]
///
/// Behaviour of nulls is governed by SetLookupOptions::skip_nulls.
///
/// \param[in] values array-like input
/// \param[in] options SetLookupOptions
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> IndexIn(const Datum& values, const SetLookupOptions& options,
                      ExecContext* ctx = NULLPTR);
ARROW_EXPORT
Result<Datum> IndexIn(const Datum& values, const Datum& value_set,
                      ExecContext* ctx = NULLPTR);

/// \brief IsValid returns true for each element of `values` that is not null,
/// false otherwise
///
/// \param[in] values input to examine for validity
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> IsValid(const Datum& values, ExecContext* ctx = NULLPTR);

/// \brief IsNull returns true for each element of `values` that is null,
/// false otherwise
///
/// \param[in] values input to examine for nullity
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> IsNull(const Datum& values, ExecContext* ctx = NULLPTR);

/// \brief IsNan returns true for each element of `values` that is NaN,
/// false otherwise
///
/// \param[in] values input to look for NaN
/// \param[in] ctx the function execution context, optional
/// \return the resulting datum
///
/// \since 3.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> IsNan(const Datum& values, ExecContext* ctx = NULLPTR);

/// \brief FillNull replaces each null element in `values`
/// with `fill_value`
///
/// \param[in] values input to examine for nullity
/// \param[in] fill_value scalar
/// \param[in] ctx the function execution context, optional
///
/// \return the resulting datum
///
/// \since 1.0.0
/// \note API not yet finalized
ARROW_EXPORT
Result<Datum> FillNull(const Datum& values, const Datum& fill_value,
                       ExecContext* ctx = NULLPTR);

}  // namespace compute
}  // namespace arrow
