#ifndef CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_ROW_H_
#define CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_ROW_H_

#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "codelab/feature_store/benchmark_encoding/feature.pb.h"

namespace hcoona {
namespace codelab {
namespace feature_store {

class Row {
 public:
  explicit Row(const idl::euclid::common::Example& example);

  const absl::btree_map<std::string, std::string>& features() const {
    return features_;
  }

  const absl::btree_map<std::string, std::vector<std::string>>& raw_features()
      const {
    return raw_features_;
  }

 private:
  absl::btree_map<std::string, std::string> features_;
  absl::btree_map<std::string, std::vector<std::string>> raw_features_;
};

}  // namespace feature_store
}  // namespace codelab
}  // namespace hcoona

#endif  // CODELAB_FEATURE_STORE_BENCHMARK_ENCODING_ROW_H_
