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

#include "one/cppdeps/commands.h"

#include <iostream>

#include "boost/process.hpp"
#include "boost/process/start_dir.hpp"
#include "boost/property_map/property_map.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"

namespace cppdeps {

namespace pt = boost::property_tree;
namespace pr = boost::process;

fs::path get_tmp_dir() {
  return std::filesystem::temp_directory_path() / "cppdeps";
}
void init_tmp_dir(const fs::path& tmpdir) {
  std::filesystem::create_directory(tmpdir);
}
void cleanup_tmp_dir(const fs::path& tmpdir) {
  std::filesystem::remove_all(tmpdir);
}

void configure_cmake(const fs::path& tmpdir, const fs::path& sourcedir) {
  pr::system("cmake " + sourcedir.string(), pr::start_dir = tmpdir.string());
}

Files read_compile_commands(const fs::path& filename) {
  Files files;
  pt::ptree tree;
  pt::read_json(filename, tree);

  for (const auto& entry : tree) {
    files.emplace_back(File{entry.second.get<std::string>("file"),
                            entry.second.get<std::string>("directory"),
                            entry.second.get<std::string>("command")});
  }
  return files;
}

std::vector<std::string> execute_file(const File& file) {
  std::vector<std::string> output;

  pr::ipstream stream;
  pr::child c(file.command + " -E -H", pr::std_err > stream,
              pr::start_dir = file.directory.string());

  std::string line;
  while (stream && std::getline(stream, line) && !line.empty())
    output.emplace_back(line);
  c.wait();
  return output;
}

void render_dot_file(const fs::path& filename) {
  pr::system("neato -Tpdf -O " + filename.string());
}

}  // namespace cppdeps
