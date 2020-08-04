#include "codelab/feature_store/benchmark_encoding/row.h"

#include <string>
#include <utility>
#include <vector>

namespace codelab {
namespace feature_store {

Row::Row(const idl::euclid::common::Example& example) {
  for (const idl::euclid::common::NamedFeature& named_feature :
       example.named_feature()) {
    features_.emplace(named_feature.name(),
                      named_feature.feature().SerializeAsString());
  }

  for (const idl::euclid::common::NamedRawFeature& named_raw_feature :
       example.named_raw_feature()) {
    std::vector<std::string> features;
    features.reserve(named_raw_feature.raw_feature().feature_size());
    for (const idl::euclid::common::Feature& feature :
         named_raw_feature.raw_feature().feature()) {
      features.emplace_back(feature.SerializeAsString());
    }

    raw_features_.emplace(named_raw_feature.name(), std::move(features));
  }
}

}  // namespace feature_store
}  // namespace codelab
