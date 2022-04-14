// Copyright (c) 2022 Gereon Kremer<gkremer@stanford.edu> and
// Zhang Shuai<zhangshuai.ustc@gmail.com>. All rights reserved.
//
// This file is part of ONE.
//
// ONE is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// ONE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ONE. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace cppdeps {

namespace fs = std::filesystem;

struct BoostGraph;

class Graph {
  std::unique_ptr<BoostGraph> mGraph;
  std::map<std::string, std::size_t> mVertices;
  std::mutex mMutex;
  std::vector<std::string> mExcludes;
  std::string mSource;
  std::string mTmp;

  void clean_path(fs::path& filename) const;
  bool consider_file(const fs::path& filename);

 public:
  Graph(const std::vector<std::string>& excludes, const std::string& sourcedir,
        const std::string& tmpdir);
  ~Graph() noexcept;
  void parse_output(const std::vector<std::string>& output,
                    const std::string& filename);
  void clean();
  void analyze_components();
  void layout();
  void write_graphviz(const fs::path& filename = "") const;
};

}  // namespace cppdeps
