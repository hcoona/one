/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "folly/json.h"
#include "folly/json_patch.h"
#include "folly/json_pointer.h"
#include "folly/portability/GMock.h"
#include "folly/portability/GTest.h"

using folly::dynamic;
using folly::json_patch;
using folly::json_pointer;

using err_code = folly::json_patch::parse_error_code;
using op_code = folly::json_patch::patch_operation_code;

class JsonPatchTest : public ::testing::Test {};

TEST_F(JsonPatchTest, ValidPatch) {
  // from RFC 6902
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "test", "path": "/a/b/c", "value": "foo" },
     { "op": "remove", "path": "/a/b/c" },
     { "op": "add", "path": "/a/b/c", "value": [ "foo", "bar" ] },
     { "op": "replace", "path": "/a/b/c", "value": 42 },
     { "op": "move", "from": "/a/b/c", "path": "/a/b/d" },
     { "op": "copy", "from": "/a/b/d", "path": "/a/b/e" }
   ])";
  auto const expected = std::vector<json_patch::patch_operation>{
      {op_code::test,
       json_pointer::parse("/a/b/c"),
       folly::none,
       dynamic("foo")},
      {op_code::remove,
       json_pointer::parse("/a/b/c"),
       folly::none,
       folly::none},
      {op_code::add,
       json_pointer::parse("/a/b/c"),
       folly::none,
       folly::parseJson(R"(["foo", "bar"])")},
      {op_code::replace,
       json_pointer::parse("/a/b/c"),
       folly::none,
       dynamic(42)},
      {op_code::move,
       json_pointer::parse("/a/b/d"),
       json_pointer::parse("/a/b/c"),
       folly::none},
      {op_code::copy,
       json_pointer::parse("/a/b/e"),
       json_pointer::parse("/a/b/d"),
       folly::none}};
  auto const parsed =
      json_patch::try_parse(folly::parseJson(jsonPatchStr)).value().ops();
  EXPECT_EQ(expected, parsed);
}

TEST_F(JsonPatchTest, InvalidPatches) {
  EXPECT_EQ(
      err_code::invalid_shape,
      json_patch::try_parse(dynamic::object()).error().error_code);

  EXPECT_EQ(
      err_code::invalid_shape,
      json_patch::try_parse(dynamic::array(dynamic::array()))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::missing_op,
      json_patch::try_parse(folly::parseJson(R"([{"path": "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::unknown_op,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "blah", "path": "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::malformed_op,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": ["blah"], "path": "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::missing_path_attr,
      json_patch::try_parse(folly::parseJson(R"([{"op": "test"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::malformed_path_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "test", "path" : "a/z/x"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::malformed_path_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "test", "path" : ["a/z/x"]}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::missing_from_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "copy", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::malformed_from_attr,
      json_patch::try_parse(
          folly::parseJson(
              R"([{"op": "copy", "from" : "c/d/e", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::overlapping_pointers,
      json_patch::try_parse(
          folly::parseJson(
              R"([{"op": "move", "from" : "/a/b/c", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::overlapping_pointers,
      json_patch::try_parse(
          folly::parseJson(
              R"([{"op": "move", "from" : "/a/b/c", "path" : "/a/b/c/d"}])"))
          .error()
          .error_code);

  // validate presence of mandatory per-operation attributes

  EXPECT_EQ(
      err_code::missing_value_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "test", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::missing_value_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "replace", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::missing_from_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "move", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  EXPECT_EQ(
      err_code::missing_from_attr,
      json_patch::try_parse(
          folly::parseJson(R"([{"op": "copy", "path" : "/a/b/c"}])"))
          .error()
          .error_code);

  // test the object reference in error: in patch below, 3rd entry is incorrect

  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "test", "path": "/a/b/c", "value": "foo" },
     { "op": "remove", "path": "/a/b/c" },
     { "op": "add", "path": "/a/b/c" }
   ])";
  auto jsonObj = folly::parseJson(jsonPatchStr);
  auto err = json_patch::try_parse(jsonObj).error();
  EXPECT_EQ(err_code::missing_value_attr, err.error_code);
  // the invalid entry - check pointers and values they point at
  EXPECT_EQ(&jsonObj[2], err.obj);
  EXPECT_EQ(jsonObj[2], *err.obj);
}

TEST_F(JsonPatchTest, SuccessfulPatchApplication) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "test", "path": "/a/b/c", "value": "foo" },
     { "op": "remove", "path": "/a/b/c" },
     { "op": "add", "path": "/a/b/c", "value": [ "foo", "bar" ] },
     { "op": "replace", "path": "/a/b/c", "value": 42 },
     { "op": "test", "path": "/a/b/c", "value": 42 },
     { "op": "move", "from": "/a/b/c", "path": "/a/b/d" },
     { "op": "copy", "from": "/a/b/d", "path": "/a/b/e" },
     { "op": "add", "path": "/a/b/c", "value": [1, 2, 3] },
     { "op": "add", "path": "/a/b/c/1", "value": 100 }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::object("c", "foo")));

  auto res = patch.apply(objToMutate);

  EXPECT_FALSE(res.hasError()) << (int)res.error().error_code;

  EXPECT_EQ(42, objToMutate["a"]["b"]["d"].asInt());
  EXPECT_EQ(42, objToMutate["a"]["b"]["e"].asInt());
  EXPECT_EQ(100, objToMutate["a"]["b"]["c"][1].asInt());
  EXPECT_EQ(1, objToMutate["a"]["b"]["c"][0].asInt());
}

TEST_F(JsonPatchTest, TestOpFailure) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "test", "path": "/a/b/c", "value": "foo" },
     { "op": "remove", "path": "/a/b/c" }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::object("c", "bar")));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(
      json_patch::patch_application_error_code::test_failed,
      res.error().error_code);

  EXPECT_EQ("bar", objToMutate["a"]["b"]["c"].asString());
}

TEST_F(JsonPatchTest, PathNotFound) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "remove", "path": "/a/b/d" }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::object("c", "foo")));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(
      json_patch::patch_application_error_code::path_not_found,
      res.error().error_code);

  EXPECT_EQ("foo", objToMutate["a"]["b"]["c"].asString());
}

TEST_F(JsonPatchTest, FromNotFound) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "copy", "from": "/a/c/b", "path": "/a/b/d" }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::object("c", "foo")));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(
      json_patch::patch_application_error_code::from_not_found,
      res.error().error_code);

  EXPECT_THROW(objToMutate["a"]["b"].at("d"), std::out_of_range);
}

TEST_F(JsonPatchTest, RemoveRootObject) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "remove", "path": "" }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::object("c", "foo")));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(
      json_patch::patch_application_error_code::other, res.error().error_code);

  EXPECT_EQ("foo", objToMutate["a"]["b"]["c"].asString());
}

TEST_F(JsonPatchTest, AddWithAppend) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "add", "path": "/a/b/-", "value": 100 }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::array(1, 2, 3, 4)));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasValue());
  EXPECT_EQ(100, objToMutate["a"]["b"][4].asInt());
}

TEST_F(JsonPatchTest, RemoveWithMinus) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "remove", "path": "/a/b/-" }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::array(1, 2, 3, 4)));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(
      json_patch::patch_application_error_code::path_not_found,
      res.error().error_code);
}

TEST_F(JsonPatchTest, FailedOpIndex) {
  constexpr folly::StringPiece jsonPatchStr = R"(
   [
     { "op": "test", "path": "/a/b/0", "value": 1 },
     { "op": "test", "path": "/a/b/0", "value": 2 }
   ])";
  auto patch = json_patch::try_parse(folly::parseJson(jsonPatchStr)).value();

  dynamic objToMutate =
      dynamic::object("a", dynamic::object("b", dynamic::array(1, 2, 3, 4)));

  auto res = patch.apply(objToMutate);

  EXPECT_TRUE(res.hasError());
  EXPECT_EQ(
      json_patch::patch_application_error_code::test_failed,
      res.error().error_code);
  EXPECT_EQ(1, res.error().index);
}
