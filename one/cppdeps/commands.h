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
#include <string>
#include <vector>

namespace cppdeps {

namespace fs = std::filesystem;

struct File {
	fs::path file;
	fs::path directory;
	std::string command;
};
using Files = std::vector<File>;

fs::path get_tmp_dir();
void init_tmp_dir(const fs::path& tmpdir);
void cleanup_tmp_dir(const fs::path& tmpdir);
void configure_cmake(const fs::path& tmpdir, const fs::path& sourcedir);
Files read_compile_commands(const fs::path& filename);
std::vector<std::string> execute_file(const File& file);
void render_dot_file(const fs::path& filename);

}
