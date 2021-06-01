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

#include "third_party/arrow/src/arrow/compute/api_aggregate.h"

#include "third_party/arrow/src/arrow/compute/exec.h"

namespace arrow {
namespace compute {

// ----------------------------------------------------------------------
// Scalar aggregates

Result<Datum> Count(const Datum& value, CountOptions options, ExecContext* ctx) {
  return CallFunction("count", {value}, &options, ctx);
}

Result<Datum> Mean(const Datum& value, ExecContext* ctx) {
  return CallFunction("mean", {value}, ctx);
}

Result<Datum> Sum(const Datum& value, ExecContext* ctx) {
  return CallFunction("sum", {value}, ctx);
}

Result<Datum> MinMax(const Datum& value, const MinMaxOptions& options, ExecContext* ctx) {
  return CallFunction("min_max", {value}, &options, ctx);
}

Result<Datum> Any(const Datum& value, ExecContext* ctx) {
  return CallFunction("any", {value}, ctx);
}

Result<Datum> All(const Datum& value, ExecContext* ctx) {
  return CallFunction("all", {value}, ctx);
}

Result<Datum> Mode(const Datum& value, const ModeOptions& options, ExecContext* ctx) {
  return CallFunction("mode", {value}, &options, ctx);
}

Result<Datum> Stddev(const Datum& value, const VarianceOptions& options,
                     ExecContext* ctx) {
  return CallFunction("stddev", {value}, &options, ctx);
}

Result<Datum> Variance(const Datum& value, const VarianceOptions& options,
                       ExecContext* ctx) {
  return CallFunction("variance", {value}, &options, ctx);
}

Result<Datum> Quantile(const Datum& value, const QuantileOptions& options,
                       ExecContext* ctx) {
  return CallFunction("quantile", {value}, &options, ctx);
}

Result<Datum> TDigest(const Datum& value, const TDigestOptions& options,
                      ExecContext* ctx) {
  return CallFunction("tdigest", {value}, &options, ctx);
}

}  // namespace compute
}  // namespace arrow
