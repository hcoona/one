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

#include "one/cppdeps/graph.h"

#include <regex>

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/circle_layout.hpp"
#include "boost/graph/fruchterman_reingold.hpp"
#include "boost/graph/graph_traits.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/graph/labeled_graph.hpp"
#include "boost/graph/point_traits.hpp"
#include "boost/graph/strong_components.hpp"

namespace cppdeps {

std::regex line_regex("(\\.+) (.+)");

std::size_t longest_common_prefix(const std::string& s, const std::string& t) {
  std::size_t res = 0;
  while (res < s.size() && res < t.size() && s[res] == t[res]) ++res;
  return res;
}

struct VertexData {
  std::string name{};
  boost::square_topology<>::point point{};
  int component{};
};
struct BoostGraph : public boost::adjacency_list<boost::vecS, boost::vecS,
                                                 boost::directedS, VertexData> {
};

template <typename G>
struct vertex_property_writer {
  template <typename VertexOrEdge>
  void operator()(std::ostream& out, const VertexOrEdge& v) const {
    out << "[";
    out << "shape=box, ";
    out << "label=" << boost::escape_dot_string(graph[v].name) << ", ";
    out << "pos=\"" << graph[v].point[0] << "," << graph[v].point[1] << "!\"";
    out << "]";
  }
  const G& graph;
};
template <typename G>
auto make_vertex_property_writer(const G& g) {
  return vertex_property_writer<G>{g};
}

void Graph::clean_path(fs::path& filename) const {
  std::string s = filename.string();
  if (s.find(mTmp) == 0) {
    filename = fs::path(mSource + s.substr(mTmp.size() + 1));
  }
}

bool Graph::consider_file(const fs::path& filename) {
  const auto name = filename.string();
  for (const auto& pattern : mExcludes) {
    if (name.find(pattern) != std::string::npos) return false;
  }
  return true;
}

Graph::Graph(const std::vector<std::string>& excludes,
             const std::string& sourcedir, const std::string& tmpdir)
    : mGraph(std::make_unique<BoostGraph>()),
      mExcludes(excludes),
      mSource(sourcedir),
      mTmp(tmpdir) {}
Graph::~Graph() noexcept = default;

void Graph::parse_output(const std::vector<std::string>& output,
                         const std::string& filename) {
  std::vector<std::string> stack({filename});

  auto get_vertex = [this](const std::string& path) {
    auto it = mVertices.find(path);
    if (it != mVertices.end()) return it->second;
    auto vertex = boost::add_vertex(VertexData{path}, *mGraph);
    mVertices.emplace(path, vertex);
    return vertex;
  };

  for (const auto& o : output) {
    std::smatch match;
    if (std::regex_match(o, match, line_regex)) {
      std::size_t indent = match[1].length();
      assert(indent <= stack.size());
      if (indent == stack.size()) {
        stack.emplace_back(match[2]);
      } else {
        while (indent < stack.size() - 1) {
          stack.pop_back();
        }
        stack.back() = match[2];
      }

      auto source_file = fs::canonical(stack[stack.size() - 2]);
      auto target_file = fs::canonical(stack[stack.size() - 1]);
      clean_path(source_file);
      clean_path(target_file);

      if (consider_file(source_file) && consider_file(target_file)) {
        std::lock_guard<std::mutex> guard(mMutex);
        auto source = get_vertex(source_file.string());
        auto target = get_vertex(target_file.string());
        if (!boost::edge(source, target, *mGraph).second) {
          boost::add_edge(source, target, *mGraph);
        }
      }
    }
  }
}

void Graph::clean() {
  const auto& vert = boost::vertices(*mGraph);
  if (vert.first == vert.second) return;
  std::string cur_prefix = (*mGraph)[*vert.first].name;
  std::size_t prefix_length = cur_prefix.size();
  for (auto it = vert.first; it != vert.second; ++it) {
    prefix_length = std::min(
        prefix_length, longest_common_prefix(cur_prefix, (*mGraph)[*it].name));
  }
  for (auto it = vert.first; it != vert.second; ++it) {
    (*mGraph)[*it].name = (*mGraph)[*it].name.substr(prefix_length);
  }
}

void Graph::analyze_components() {
  std::vector<BoostGraph::edge_descriptor> tmp_edges;
  auto edges = boost::edges(*mGraph);
  for (auto it = edges.first; it != edges.second; ++it) {
    auto pair = boost::add_edge(boost::target(*it, *mGraph),
                                boost::source(*it, *mGraph), *mGraph);
    tmp_edges.push_back(pair.first);
  }

  auto res = boost::strong_components(
      *mGraph, boost::get(&VertexData::component, *mGraph));
  (void)res;

  for (const auto& e : tmp_edges) {
    boost::remove_edge(e, *mGraph);
  }

  const auto& vert = boost::vertices(*mGraph);
  for (auto it = vert.first; it != vert.second; ++it) {
    std::cout << (*mGraph)[*it].name << " -> " << (*mGraph)[*it].component
              << std::endl;
  }
}

void Graph::layout() {
  boost::circle_graph_layout(*mGraph, boost::get(&VertexData::point, *mGraph),
                             15.0);

  boost::fruchterman_reingold_force_directed_layout(
      *mGraph, boost::get(&VertexData::point, *mGraph),
      boost::square_topology<>(30.0),
      boost::cooling(boost::linear_cooling<double>(200)));
}

void Graph::write_graphviz(const fs::path& filename) const {
  if (filename == "") {
    boost::write_graphviz(std::cout, *mGraph,
                          make_vertex_property_writer((*mGraph)));
  } else {
    std::ofstream out(filename.string());
    boost::write_graphviz(out, *mGraph, make_vertex_property_writer((*mGraph)));
    out.close();
  }
}

}  // namespace cppdeps
