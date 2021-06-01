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

#include <memory>

#include "third_party/googletest/googlemock/include/gmock/gmock.h"
#include "third_party/googletest/googletest/include/gtest/gtest.h"

#ifdef ARROW_WITH_UTF8PROC
#include <utf8proc.h>
#endif

#include "third_party/arrow/src/arrow/compute/api_scalar.h"
#include "third_party/arrow/src/arrow/compute/kernels/test_util.h"
#include "third_party/arrow/src/arrow/testing/gtest_util.h"

namespace arrow {
namespace compute {

// interesting utf8 characters for testing (lower case / upper case):
//  * ῦ / Υ͂ (3 to 4 code units) (Note, we don't support this yet, utf8proc does not use
//  SpecialCasing.txt)
//  * ɑ /  Ɑ (2 to 3 code units)
//  * ı / I (2 to 1 code units)
//  * Ⱥ / ⱥ  (2 to 3 code units)

template <typename TestType>
class BaseTestStringKernels : public ::testing::Test {
 protected:
  using OffsetType = typename TypeTraits<TestType>::OffsetType;

  void CheckUnary(std::string func_name, std::string json_input,
                  std::shared_ptr<DataType> out_ty, std::string json_expected,
                  const FunctionOptions* options = nullptr) {
    CheckScalarUnary(func_name, type(), json_input, out_ty, json_expected, options);
  }

  void CheckBinaryScalar(std::string func_name, std::string json_left_input,
                         std::string json_right_scalar, std::shared_ptr<DataType> out_ty,
                         std::string json_expected,
                         const FunctionOptions* options = nullptr) {
    CheckScalarBinaryScalar(func_name, type(), json_left_input, json_right_scalar, out_ty,
                            json_expected, options);
  }

  std::shared_ptr<DataType> type() { return TypeTraits<TestType>::type_singleton(); }

  std::shared_ptr<DataType> offset_type() {
    return TypeTraits<OffsetType>::type_singleton();
  }
};

template <typename TestType>
class TestBinaryKernels : public BaseTestStringKernels<TestType> {};

TYPED_TEST_SUITE(TestBinaryKernels, BinaryTypes);

TYPED_TEST(TestBinaryKernels, BinaryLength) {
  this->CheckUnary("binary_length", R"(["aaa", null, "áéíóú", "", "b"])",
                   this->offset_type(), "[3, null, 10, 0, 1]");
}

template <typename TestType>
class TestStringKernels : public BaseTestStringKernels<TestType> {};

TYPED_TEST_SUITE(TestStringKernels, StringTypes);

TYPED_TEST(TestStringKernels, AsciiUpper) {
  this->CheckUnary("ascii_upper", "[]", this->type(), "[]");
  this->CheckUnary("ascii_upper", "[\"aAazZæÆ&\", null, \"\", \"bbb\"]", this->type(),
                   "[\"AAAZZæÆ&\", null, \"\", \"BBB\"]");
}

TYPED_TEST(TestStringKernels, AsciiLower) {
  this->CheckUnary("ascii_lower", "[]", this->type(), "[]");
  this->CheckUnary("ascii_lower", "[\"aAazZæÆ&\", null, \"\", \"BBB\"]", this->type(),
                   "[\"aaazzæÆ&\", null, \"\", \"bbb\"]");
}

TEST(TestStringKernels, LARGE_MEMORY_TEST(Utf8Upper32bitGrowth)) {
  // 0x7fff * 0xffff is the max a 32 bit string array can hold
  // since the utf8_upper kernel can grow it by 3/2, the max we should accept is is
  // 0x7fff * 0xffff * 2/3 = 0x5555 * 0xffff, so this should give us a CapacityError
  std::string str(0x5556 * 0xffff, 'a');
  arrow::StringBuilder builder;
  ASSERT_OK(builder.Append(str));
  std::shared_ptr<arrow::Array> array;
  arrow::Status st = builder.Finish(&array);
  const FunctionOptions* options = nullptr;
  EXPECT_RAISES_WITH_MESSAGE_THAT(CapacityError,
                                  testing::HasSubstr("Result might not fit"),
                                  CallFunction("utf8_upper", {array}, options));
  ASSERT_OK_AND_ASSIGN(auto scalar, array->GetScalar(0));
  EXPECT_RAISES_WITH_MESSAGE_THAT(CapacityError,
                                  testing::HasSubstr("Result might not fit"),
                                  CallFunction("utf8_upper", {scalar}, options));
}

TYPED_TEST(TestStringKernels, Utf8Length) {
  this->CheckUnary("utf8_length",
                   R"(["aaa", null, "áéíóú", "ɑɽⱤoW😀", "áéí 0😀", "", "b"])",
                   this->offset_type(), "[3, null, 5, 6, 6, 0, 1]");
}

#ifdef ARROW_WITH_UTF8PROC

TYPED_TEST(TestStringKernels, Utf8Upper) {
  this->CheckUnary("utf8_upper", "[\"aAazZæÆ&\", null, \"\", \"b\"]", this->type(),
                   "[\"AAAZZÆÆ&\", null, \"\", \"B\"]");

  // test varying encoding lenghts and thus changing indices/offsets
  this->CheckUnary("utf8_upper", "[\"ɑɽⱤoW\", null, \"ıI\", \"b\"]", this->type(),
                   "[\"ⱭⱤⱤOW\", null, \"II\", \"B\"]");

  // ῦ to Υ͂ not supported
  // this->CheckUnary("utf8_upper", "[\"ῦɐɜʞȿ\"]", this->type(),
  // "[\"Υ͂ⱯꞫꞰⱾ\"]");

  // test maximum buffer growth
  this->CheckUnary("utf8_upper", "[\"ɑɑɑɑ\"]", this->type(), "[\"ⱭⱭⱭⱭ\"]");

  // Test invalid data
  auto invalid_input = ArrayFromJSON(this->type(), "[\"ɑa\xFFɑ\", \"ɽ\xe1\xbdɽaa\"]");
  EXPECT_RAISES_WITH_MESSAGE_THAT(Invalid, testing::HasSubstr("Invalid UTF8 sequence"),
                                  CallFunction("utf8_upper", {invalid_input}));
}

TYPED_TEST(TestStringKernels, Utf8Lower) {
  this->CheckUnary("utf8_lower", "[\"aAazZæÆ&\", null, \"\", \"b\"]", this->type(),
                   "[\"aaazzææ&\", null, \"\", \"b\"]");

  // test varying encoding lengths and thus changing indices/offsets
  this->CheckUnary("utf8_lower", "[\"ⱭɽⱤoW\", null, \"ıI\", \"B\"]", this->type(),
                   "[\"ɑɽɽow\", null, \"ıi\", \"b\"]");

  // ῦ to Υ͂ is not supported, but in principle the reverse is, but it would need
  // normalization
  // this->CheckUnary("utf8_lower", "[\"Υ͂ⱯꞫꞰⱾ\"]", this->type(),
  // "[\"ῦɐɜʞȿ\"]");

  // test maximum buffer growth
  this->CheckUnary("utf8_lower", "[\"ȺȺȺȺ\"]", this->type(), "[\"ⱥⱥⱥⱥ\"]");

  // Test invalid data
  auto invalid_input = ArrayFromJSON(this->type(), "[\"Ⱥa\xFFⱭ\", \"Ɽ\xe1\xbdⱤaA\"]");
  EXPECT_RAISES_WITH_MESSAGE_THAT(Invalid, testing::HasSubstr("Invalid UTF8 sequence"),
                                  CallFunction("utf8_lower", {invalid_input}));
}

TYPED_TEST(TestStringKernels, IsAlphaNumericUnicode) {
  // U+08BE (utf8: 	\xE0\xA2\xBE) is undefined, but utf8proc things it is
  // UTF8PROC_CATEGORY_LO
  this->CheckUnary("utf8_is_alnum", "[\"ⱭɽⱤoW123\", null, \"Ɑ2\", \"!\", \"\"]",
                   boolean(), "[true, null, true, false, false]");
}

TYPED_TEST(TestStringKernels, IsAlphaUnicode) {
  // U+08BE (utf8: 	\xE0\xA2\xBE) is undefined, but utf8proc things it is
  // UTF8PROC_CATEGORY_LO
  this->CheckUnary("utf8_is_alpha", "[\"ⱭɽⱤoW\", null, \"Ɑ2\", \"!\", \"\"]", boolean(),
                   "[true, null, false, false, false]");
}

TYPED_TEST(TestStringKernels, IsAscii) {
  this->CheckUnary("string_is_ascii", "[\"azAZ~\", null, \"Ɑ\", \"\"]", boolean(),
                   "[true, null, false, true]");
}

TYPED_TEST(TestStringKernels, IsDecimalUnicode) {
  // ٣ is arabic 3 (decimal), Ⅳ roman (non-decimal)
  this->CheckUnary("utf8_is_decimal", "[\"12\", null, \"٣\", \"Ⅳ\", \"1a\", \"\"]",
                   boolean(), "[true, null, true, false, false, false]");
}

TYPED_TEST(TestStringKernels, IsDigitUnicode) {
  // These are digits according to Python, but we don't have the information in
  // utf8proc for this
  // this->CheckUnary("utf8_is_digit", "[\"²\", \"①\"]", boolean(), "[true,
  // true]");
}

TYPED_TEST(TestStringKernels, IsNumericUnicode) {
  // ٣ is arabic 3 (decimal), Ⅳ roman (non-decimal)
  this->CheckUnary("utf8_is_numeric", "[\"12\", null, \"٣\", \"Ⅳ\", \"1a\", \"\"]",
                   boolean(), "[true, null, true, true, false, false]");
  // These are numerical according to Python, but we don't have the information in
  // utf8proc for this
  // this->CheckUnary("utf8_is_numeric", "[\"㐅\", \"卌\"]", boolean(),
  //                  "[true, null, true, true, false, false]");
}

TYPED_TEST(TestStringKernels, IsLowerUnicode) {
  // ٣ is arabic 3 (decimal), Φ capital
  this->CheckUnary("utf8_is_lower",
                   "[\"12\", null, \"٣a\", \"٣A\", \"1a\", \"Φ\", \"\", \"with space\", "
                   "\"With space\"]",
                   boolean(),
                   "[false, null, true, false, true, false, false, true, false]");
  // lower case character utf8proc does not know about
  // this->CheckUnary("utf8_is_lower", "[\"ª\", \"ₕ\"]", boolean(), "[true,
  // true]");
}

TYPED_TEST(TestStringKernels, IsPrintableUnicode) {
  // U+2008 (utf8: \xe2\x80\x88) is punctuation space, it is NOT printable
  // U+0378 (utf8: \xCD\xB8) is an undefined char, it has no category
  this->CheckUnary(
      "utf8_is_printable",
      "[\" 123azAZ!~\", null, \"\xe2\x80\x88\", \"\", \"\\r\", \"\xCD\xB8\"]", boolean(),
      "[true, null, false, true, false, false]");
}

TYPED_TEST(TestStringKernels, IsSpaceUnicode) {
  // U+2008 (utf8: \xe2\x80\x88) is punctuation space
  this->CheckUnary("utf8_is_space", "[\" \", null, \"  \", \"\\t\\r\"]", boolean(),
                   "[true, null, true, true]");
  this->CheckUnary("utf8_is_space", "[\" a\", null, \"a \", \"~\", \"\xe2\x80\x88\"]",
                   boolean(), "[false, null, false, false, true]");
}

TYPED_TEST(TestStringKernels, IsTitleUnicode) {
  // ٣ is arabic 3 (decimal), Φ capital
  this->CheckUnary("utf8_is_title",
                   "[\"Is\", null, \"Is Title\", \"Is٣Title\", \"Is_Ǆ\", \"Φ\", \"Ǆ\"]",
                   boolean(), "[true, null, true, true, true, true, true]");
  this->CheckUnary(
      "utf8_is_title",
      "[\"IsN\", null, \"IsNoTitle\", \"Is No T٣tle\", \"IsǄ\", \"ΦΦ\", \"ǆ\", \"_\"]",
      boolean(), "[false, null, false, false, false, false, false, false]");
}

// Older versions of utf8proc fail
#if !(UTF8PROC_VERSION_MAJOR <= 2 && UTF8PROC_VERSION_MINOR < 5)

TYPED_TEST(TestStringKernels, IsUpperUnicode) {
  // ٣ is arabic 3 (decimal), Φ capital
  this->CheckUnary("utf8_is_upper",
                   "[\"12\", null, \"٣a\", \"٣A\", \"1A\", \"Φ\", \"\", \"Ⅰ\", \"Ⅿ\"]",
                   boolean(),
                   "[false, null, false, true, true, true, false, true, true]");
  // * Ⅰ to Ⅿ is a special case (roman capital), as well as Ⓐ to Ⓩ
  // * ϒ - \xCF\x92 - Greek Upsilon with Hook Symbol - upper case, but has no direct lower
  // case
  // * U+1F88 - ᾈ - \E1\xBE\x88 - Greek Capital Letter Alpha with Psili and Prosgegrammeni
  // - title case
  // U+10400 - 𐐀 - \xF0x90x90x80 - Deseret Capital Letter Long - upper case
  // * U+A7BA - Ꞻ - \xEA\x9E\xBA - Latin Capital Letter Glottal A -  new in unicode 13
  // (not tested since it depends on the version of libutf8proc)
  // * U+A7BB - ꞻ - \xEA\x9E\xBB - Latin Small Letter Glottal A - new in unicode 13
  this->CheckUnary("utf8_is_upper",
                   "[\"Ⓐ\", \"Ⓩ\", \"ϒ\", \"ᾈ\", \"\xEA\x9E\xBA\", \"xF0x90x90x80\"]",
                   boolean(), "[true, true, true, false, true, false]");
}

#endif  // UTF8PROC_VERSION_MINOR >= 5

#endif  // ARROW_WITH_UTF8PROC

TYPED_TEST(TestStringKernels, IsAlphaNumericAscii) {
  this->CheckUnary("ascii_is_alnum",
                   "[\"ⱭɽⱤoW123\", null, \"Ɑ2\", \"!\", \"\", \"a space\", \"1 space\"]",
                   boolean(), "[false, null, false, false, false, false, false]");
  this->CheckUnary("ascii_is_alnum", "[\"aRoW123\", null, \"a2\", \"a\", \"2\", \"\"]",
                   boolean(), "[true, null, true, true, true, false]");
}

TYPED_TEST(TestStringKernels, IsAlphaAscii) {
  this->CheckUnary("ascii_is_alpha", "[\"ⱭɽⱤoW\", \"arrow\", null, \"a2\", \"!\", \"\"]",
                   boolean(), "[false, true, null, false, false, false]");
}

TYPED_TEST(TestStringKernels, IsDecimalAscii) {
  // ٣ is arabic 3
  this->CheckUnary("ascii_is_decimal", "[\"12\", null, \"٣\", \"Ⅳ\", \"1a\", \"\"]",
                   boolean(), "[true, null, false, false, false, false]");
}

TYPED_TEST(TestStringKernels, IsLowerAscii) {
  // ٣ is arabic 3 (decimal), φ lower greek
  this->CheckUnary("ascii_is_lower",
                   "[\"12\", null, \"٣a\", \"٣A\", \"1a\", \"φ\", \"\"]", boolean(),
                   "[false, null, true, false, true, false, false]");
}
TYPED_TEST(TestStringKernels, IsPrintableAscii) {
  // \xe2\x80\x88 is punctuation space
  this->CheckUnary("ascii_is_printable",
                   "[\" 123azAZ!~\", null, \"\xe2\x80\x88\", \"\", \"\\r\"]", boolean(),
                   "[true, null, false, true, false]");
}

TYPED_TEST(TestStringKernels, IsSpaceAscii) {
  // \xe2\x80\x88 is punctuation space
  // Note: for ascii version, the non-ascii chars are seen as non-cased
  this->CheckUnary("ascii_is_space", "[\" \", null, \"  \", \"\\t\\r\"]", boolean(),
                   "[true, null, true, true]");
  this->CheckUnary("ascii_is_space", "[\" a\", null, \"a \", \"~\", \"\xe2\x80\x88\"]",
                   boolean(), "[false, null, false, false, false]");
}

TYPED_TEST(TestStringKernels, IsTitleAscii) {
  // ٣ is arabic 3 (decimal), Φ capital
  // Note: for ascii version, the non-ascii chars are seen as non-cased
  this->CheckUnary("ascii_is_title",
                   "[\"Is\", null, \"Is Title\", \"Is٣Title\", \"Is_Ǆ\", \"Φ\", \"Ǆ\"]",
                   boolean(), "[true, null, true, true, true, false, false]");
  this->CheckUnary(
      "ascii_is_title",
      "[\"IsN\", null, \"IsNoTitle\", \"Is No T٣tle\", \"IsǄ\", \"ΦΦ\", \"ǆ\", \"_\"]",
      boolean(), "[false, null, false, false, true, false, false, false]");
}

TYPED_TEST(TestStringKernels, IsUpperAscii) {
  // ٣ is arabic 3 (decimal), Φ capital greek
  this->CheckUnary("ascii_is_upper",
                   "[\"12\", null, \"٣a\", \"٣A\", \"1A\", \"Φ\", \"\"]", boolean(),
                   "[false, null, false, true, true, false, false]");
}

TYPED_TEST(TestStringKernels, MatchSubstring) {
  MatchSubstringOptions options{"ab"};
  this->CheckUnary("match_substring", "[]", boolean(), "[]", &options);
  this->CheckUnary("match_substring", R"(["abc", "acb", "cab", null, "bac"])", boolean(),
                   "[true, false, true, null, false]", &options);

  MatchSubstringOptions options_repeated{"abab"};
  this->CheckUnary("match_substring", R"(["abab", "ab", "cababc", null, "bac"])",
                   boolean(), "[true, false, true, null, false]", &options_repeated);

  // ARROW-9460
  MatchSubstringOptions options_double_char{"aab"};
  this->CheckUnary("match_substring", R"(["aacb", "aab", "ab", "aaab"])", boolean(),
                   "[false, true, false, true]", &options_double_char);
  MatchSubstringOptions options_double_char_2{"bbcaa"};
  this->CheckUnary("match_substring", R"(["abcbaabbbcaabccabaab"])", boolean(), "[true]",
                   &options_double_char_2);
}

#ifdef ARROW_WITH_RE2
TYPED_TEST(TestStringKernels, MatchSubstringRegex) {
  MatchSubstringOptions options{"ab"};
  this->CheckUnary("match_substring_regex", "[]", boolean(), "[]", &options);
  this->CheckUnary("match_substring_regex", R"(["abc", "acb", "cab", null, "bac"])",
                   boolean(), "[true, false, true, null, false]", &options);
  MatchSubstringOptions options_repeated{"(ab){2}"};
  this->CheckUnary("match_substring_regex", R"(["abab", "ab", "cababc", null, "bac"])",
                   boolean(), "[true, false, true, null, false]", &options_repeated);
  MatchSubstringOptions options_digit{"\\d"};
  this->CheckUnary("match_substring_regex", R"(["aacb", "a2ab", "", "24"])", boolean(),
                   "[false, true, false, true]", &options_digit);
  MatchSubstringOptions options_star{"a*b"};
  this->CheckUnary("match_substring_regex", R"(["aacb", "aab", "dab", "caaab", "b", ""])",
                   boolean(), "[true, true, true, true, true, false]", &options_star);
  MatchSubstringOptions options_plus{"a+b"};
  this->CheckUnary("match_substring_regex", R"(["aacb", "aab", "dab", "caaab", "b", ""])",
                   boolean(), "[false, true, true, true, false, false]", &options_plus);

  // Unicode character semantics
  // "\pL" means: unicode category "letter"
  // (re2 interprets "\w" as ASCII-only: https://github.com/google/re2/wiki/Syntax)
  MatchSubstringOptions options_unicode{"^\\pL+$"};
  this->CheckUnary("match_substring_regex", R"(["été", "ß", "€", ""])", boolean(),
                   "[true, true, false, false]", &options_unicode);
}

TYPED_TEST(TestStringKernels, MatchSubstringRegexNoOptions) {
  Datum input = ArrayFromJSON(this->type(), "[]");
  ASSERT_RAISES(Invalid, CallFunction("match_substring_regex", {input}));
}

TYPED_TEST(TestStringKernels, MatchSubstringRegexInvalid) {
  Datum input = ArrayFromJSON(this->type(), "[null]");
  MatchSubstringOptions options{"invalid["};
  EXPECT_RAISES_WITH_MESSAGE_THAT(
      Invalid, ::testing::HasSubstr("Invalid regular expression: missing ]"),
      CallFunction("match_substring_regex", {input}, &options));
}
#endif

TYPED_TEST(TestStringKernels, SplitBasics) {
  SplitPatternOptions options{" "};
  // basics
  this->CheckUnary("split_pattern", R"(["foo bar", "foo"])", list(this->type()),
                   R"([["foo", "bar"], ["foo"]])", &options);
  this->CheckUnary("split_pattern", R"(["foo bar", "foo", null])", list(this->type()),
                   R"([["foo", "bar"], ["foo"], null])", &options);
  // edgy cases
  this->CheckUnary("split_pattern", R"(["f  o o "])", list(this->type()),
                   R"([["f", "", "o", "o", ""]])", &options);
  this->CheckUnary("split_pattern", "[]", list(this->type()), "[]", &options);
  // longer patterns
  SplitPatternOptions options_long{"---"};
  this->CheckUnary("split_pattern", R"(["-foo---bar--", "---foo---b"])",
                   list(this->type()), R"([["-foo", "bar--"], ["", "foo", "b"]])",
                   &options_long);
  SplitPatternOptions options_long_reverse{"---", -1, /*reverse=*/true};
  this->CheckUnary("split_pattern", R"(["-foo---bar--", "---foo---b"])",
                   list(this->type()), R"([["-foo", "bar--"], ["", "foo", "b"]])",
                   &options_long_reverse);
}

TYPED_TEST(TestStringKernels, SplitMax) {
  SplitPatternOptions options{"---", 2};
  SplitPatternOptions options_reverse{"---", 2, /*reverse=*/true};
  this->CheckUnary("split_pattern", R"(["foo---bar", "foo", "foo---bar------ar"])",
                   list(this->type()),
                   R"([["foo", "bar"], ["foo"], ["foo", "bar", "---ar"]])", &options);
  this->CheckUnary(
      "split_pattern", R"(["foo---bar", "foo", "foo---bar------ar"])", list(this->type()),
      R"([["foo", "bar"], ["foo"], ["foo---bar", "", "ar"]])", &options_reverse);
}

TYPED_TEST(TestStringKernels, SplitWhitespaceAscii) {
  SplitOptions options;
  SplitOptions options_max{1};
  // basics
  this->CheckUnary("ascii_split_whitespace", R"(["foo bar", "foo  bar \tba"])",
                   list(this->type()), R"([["foo", "bar"], ["foo", "bar", "ba"]])",
                   &options);
  this->CheckUnary("ascii_split_whitespace", R"(["foo bar", "foo  bar \tba"])",
                   list(this->type()), R"([["foo", "bar"], ["foo", "bar \tba"]])",
                   &options_max);
}

TYPED_TEST(TestStringKernels, SplitWhitespaceAsciiReverse) {
  SplitOptions options{-1, /*reverse=*/true};
  SplitOptions options_max{1, /*reverse=*/true};
  // basics
  this->CheckUnary("ascii_split_whitespace", R"(["foo bar", "foo  bar \tba"])",
                   list(this->type()), R"([["foo", "bar"], ["foo", "bar", "ba"]])",
                   &options);
  this->CheckUnary("ascii_split_whitespace", R"(["foo bar", "foo  bar \tba"])",
                   list(this->type()), R"([["foo", "bar"], ["foo  bar", "ba"]])",
                   &options_max);
}

TYPED_TEST(TestStringKernels, SplitWhitespaceUTF8) {
  SplitOptions options;
  SplitOptions options_max{1};
  // \xe2\x80\x88 is punctuation space
  this->CheckUnary("utf8_split_whitespace",
                   "[\"foo bar\", \"foo\xe2\x80\x88  bar \\tba\"]", list(this->type()),
                   R"([["foo", "bar"], ["foo", "bar", "ba"]])", &options);
  this->CheckUnary("utf8_split_whitespace",
                   "[\"foo bar\", \"foo\xe2\x80\x88  bar \\tba\"]", list(this->type()),
                   R"([["foo", "bar"], ["foo", "bar \tba"]])", &options_max);
}

TYPED_TEST(TestStringKernels, SplitWhitespaceUTF8Reverse) {
  SplitOptions options{-1, /*reverse=*/true};
  SplitOptions options_max{1, /*reverse=*/true};
  // \xe2\x80\x88 is punctuation space
  this->CheckUnary("utf8_split_whitespace",
                   "[\"foo bar\", \"foo\xe2\x80\x88  bar \\tba\"]", list(this->type()),
                   R"([["foo", "bar"], ["foo", "bar", "ba"]])", &options);
  this->CheckUnary("utf8_split_whitespace",
                   "[\"foo bar\", \"foo\xe2\x80\x88  bar \\tba\"]", list(this->type()),
                   "[[\"foo\", \"bar\"], [\"foo\xe2\x80\x88  bar\", \"ba\"]]",
                   &options_max);
}

TYPED_TEST(TestStringKernels, ReplaceSubstring) {
  ReplaceSubstringOptions options{"foo", "bazz"};
  this->CheckUnary("replace_substring", R"(["foo", "this foo that foo", null])",
                   this->type(), R"(["bazz", "this bazz that bazz", null])", &options);
}

TYPED_TEST(TestStringKernels, ReplaceSubstringLimited) {
  ReplaceSubstringOptions options{"foo", "bazz", 1};
  this->CheckUnary("replace_substring", R"(["foo", "this foo that foo", null])",
                   this->type(), R"(["bazz", "this bazz that foo", null])", &options);
}

TYPED_TEST(TestStringKernels, ReplaceSubstringNoOptions) {
  Datum input = ArrayFromJSON(this->type(), "[]");
  ASSERT_RAISES(Invalid, CallFunction("replace_substring", {input}));
}

#ifdef ARROW_WITH_RE2
TYPED_TEST(TestStringKernels, ReplaceSubstringRegex) {
  ReplaceSubstringOptions options_regex{"(fo+)\\s*", "\\1-bazz"};
  this->CheckUnary("replace_substring_regex", R"(["foo ", "this foo   that foo", null])",
                   this->type(), R"(["foo-bazz", "this foo-bazzthat foo-bazz", null])",
                   &options_regex);
  // make sure we match non-overlapping
  ReplaceSubstringOptions options_regex2{"(a.a)", "aba\\1"};
  this->CheckUnary("replace_substring_regex", R"(["aaaaaa"])", this->type(),
                   R"(["abaaaaabaaaa"])", &options_regex2);

  // ARROW-12774
  ReplaceSubstringOptions options_regex3{"X", "Y"};
  this->CheckUnary("replace_substring_regex",
                   R"(["A","A","A","A","A","A","A","A","A","A","A","A","A","A","A","A"])",
                   this->type(),
                   R"(["A","A","A","A","A","A","A","A","A","A","A","A","A","A","A","A"])",
                   &options_regex3);
}

TYPED_TEST(TestStringKernels, ReplaceSubstringRegexLimited) {
  // With a finite number of replacements
  ReplaceSubstringOptions options1{"foo", "bazz", 1};
  this->CheckUnary("replace_substring", R"(["foo", "this foo that foo", null])",
                   this->type(), R"(["bazz", "this bazz that foo", null])", &options1);
  ReplaceSubstringOptions options_regex1{"(fo+)\\s*", "\\1-bazz", 1};
  this->CheckUnary("replace_substring_regex", R"(["foo ", "this foo   that foo", null])",
                   this->type(), R"(["foo-bazz", "this foo-bazzthat foo", null])",
                   &options_regex1);
}

TYPED_TEST(TestStringKernels, ReplaceSubstringRegexNoOptions) {
  Datum input = ArrayFromJSON(this->type(), "[]");
  ASSERT_RAISES(Invalid, CallFunction("replace_substring_regex", {input}));
}

TYPED_TEST(TestStringKernels, ReplaceSubstringRegexInvalid) {
  Datum input = ArrayFromJSON(this->type(), R"(["foo"])");
  ReplaceSubstringOptions options{"invalid[", ""};
  EXPECT_RAISES_WITH_MESSAGE_THAT(
      Invalid, ::testing::HasSubstr("Invalid regular expression: missing ]"),
      CallFunction("replace_substring_regex", {input}, &options));

  // Capture group number out of range
  options = ReplaceSubstringOptions{"(.)", "\\9"};
  EXPECT_RAISES_WITH_MESSAGE_THAT(
      Invalid, ::testing::HasSubstr("Invalid replacement string"),
      CallFunction("replace_substring_regex", {input}, &options));
}

TYPED_TEST(TestStringKernels, ExtractRegex) {
  ExtractRegexOptions options{"(?P<letter>[ab])(?P<digit>\\d)"};
  auto type = struct_({field("letter", this->type()), field("digit", this->type())});
  this->CheckUnary("extract_regex", R"([])", type, R"([])", &options);
  this->CheckUnary(
      "extract_regex", R"(["a1", "b2", "c3", null])", type,
      R"([{"letter": "a", "digit": "1"}, {"letter": "b", "digit": "2"}, null, null])",
      &options);
  this->CheckUnary(
      "extract_regex", R"(["a1", "c3", null, "b2"])", type,
      R"([{"letter": "a", "digit": "1"}, null, null, {"letter": "b", "digit": "2"}])",
      &options);
  this->CheckUnary("extract_regex", R"(["a1", "b2"])", type,
                   R"([{"letter": "a", "digit": "1"}, {"letter": "b", "digit": "2"}])",
                   &options);
  this->CheckUnary("extract_regex", R"(["a1", "zb3z"])", type,
                   R"([{"letter": "a", "digit": "1"}, {"letter": "b", "digit": "3"}])",
                   &options);
}

TYPED_TEST(TestStringKernels, ExtractRegexNoCapture) {
  // XXX Should we accept this or is it a user error?
  ExtractRegexOptions options{"foo"};
  auto type = struct_({});
  this->CheckUnary("extract_regex", R"(["oofoo", "bar", null])", type,
                   R"([{}, null, null])", &options);
}

TYPED_TEST(TestStringKernels, ExtractRegexNoOptions) {
  Datum input = ArrayFromJSON(this->type(), "[]");
  ASSERT_RAISES(Invalid, CallFunction("extract_regex", {input}));
}

TYPED_TEST(TestStringKernels, ExtractRegexInvalid) {
  Datum input = ArrayFromJSON(this->type(), "[]");
  ExtractRegexOptions options{"invalid["};
  EXPECT_RAISES_WITH_MESSAGE_THAT(
      Invalid, ::testing::HasSubstr("Invalid regular expression: missing ]"),
      CallFunction("extract_regex", {input}, &options));

  options = ExtractRegexOptions{"(.)"};
  EXPECT_RAISES_WITH_MESSAGE_THAT(
      Invalid, ::testing::HasSubstr("Regular expression contains unnamed groups"),
      CallFunction("extract_regex", {input}, &options));
}

#endif

TYPED_TEST(TestStringKernels, Strptime) {
  std::string input1 = R"(["5/1/2020", null, "12/11/1900"])";
  std::string output1 = R"(["2020-05-01", null, "1900-12-11"])";
  StrptimeOptions options("%m/%d/%Y", TimeUnit::MICRO);
  this->CheckUnary("strptime", input1, timestamp(TimeUnit::MICRO), output1, &options);
}

TYPED_TEST(TestStringKernels, StrptimeDoesNotProvideDefaultOptions) {
  auto input = ArrayFromJSON(this->type(), R"(["2020-05-01", null, "1900-12-11"])");
  ASSERT_RAISES(Invalid, CallFunction("strptime", {input}));
}

#ifdef ARROW_WITH_UTF8PROC

TYPED_TEST(TestStringKernels, TrimWhitespaceUTF8) {
  // \xe2\x80\x88 is punctuation space
  this->CheckUnary("utf8_trim_whitespace",
                   "[\" \\tfoo\", null, \"bar  \", \" \xe2\x80\x88 foo bar \"]",
                   this->type(), "[\"foo\", null, \"bar\", \"foo bar\"]");
  this->CheckUnary("utf8_rtrim_whitespace",
                   "[\" \\tfoo\", null, \"bar  \", \" \xe2\x80\x88 foo bar \"]",
                   this->type(),
                   "[\" \\tfoo\", null, \"bar\", \" \xe2\x80\x88 foo bar\"]");
  this->CheckUnary("utf8_ltrim_whitespace",
                   "[\" \\tfoo\", null, \"bar  \", \" \xe2\x80\x88 foo bar \"]",
                   this->type(), "[\"foo\", null, \"bar  \", \"foo bar \"]");
}

TYPED_TEST(TestStringKernels, TrimUTF8) {
  TrimOptions options{"ȺA"};
  this->CheckUnary("utf8_trim", "[\"ȺȺfooȺAȺ\", null, \"barȺAȺ\", \"ȺAȺfooȺAȺbarA\"]",
                   this->type(), "[\"foo\", null, \"bar\", \"fooȺAȺbar\"]", &options);
  this->CheckUnary("utf8_ltrim", "[\"ȺȺfooȺAȺ\", null, \"barȺAȺ\", \"ȺAȺfooȺAȺbarA\"]",
                   this->type(), "[\"fooȺAȺ\", null, \"barȺAȺ\", \"fooȺAȺbarA\"]",
                   &options);
  this->CheckUnary("utf8_rtrim", "[\"ȺȺfooȺAȺ\", null, \"barȺAȺ\", \"ȺAȺfooȺAȺbarA\"]",
                   this->type(), "[\"ȺȺfoo\", null, \"bar\", \"ȺAȺfooȺAȺbar\"]",
                   &options);

  TrimOptions options_invalid{"ɑa\xFFɑ"};
  auto input = ArrayFromJSON(this->type(), "[\"foo\"]");
  EXPECT_RAISES_WITH_MESSAGE_THAT(Invalid, testing::HasSubstr("Invalid UTF8"),
                                  CallFunction("utf8_trim", {input}, &options_invalid));
}
#endif

TYPED_TEST(TestStringKernels, TrimWhitespaceAscii) {
  // \xe2\x80\x88 is punctuation space
  this->CheckUnary("ascii_trim_whitespace",
                   "[\" \\tfoo\", null, \"bar  \", \" \xe2\x80\x88 foo bar \"]",
                   this->type(), "[\"foo\", null, \"bar\", \"\xe2\x80\x88 foo bar\"]");
  this->CheckUnary("ascii_rtrim_whitespace",
                   "[\" \\tfoo\", null, \"bar  \", \" \xe2\x80\x88 foo bar \"]",
                   this->type(),
                   "[\" \\tfoo\", null, \"bar\", \" \xe2\x80\x88 foo bar\"]");
  this->CheckUnary("ascii_ltrim_whitespace",
                   "[\" \\tfoo\", null, \"bar  \", \" \xe2\x80\x88 foo bar \"]",
                   this->type(), "[\"foo\", null, \"bar  \", \"\xe2\x80\x88 foo bar \"]");
}

TYPED_TEST(TestStringKernels, TrimAscii) {
  TrimOptions options{"BA"};
  this->CheckUnary("ascii_trim", "[\"BBfooBAB\", null, \"barBAB\", \"BABfooBABbarA\"]",
                   this->type(), "[\"foo\", null, \"bar\", \"fooBABbar\"]", &options);
  this->CheckUnary("ascii_ltrim", "[\"BBfooBAB\", null, \"barBAB\", \"BABfooBABbarA\"]",
                   this->type(), "[\"fooBAB\", null, \"barBAB\", \"fooBABbarA\"]",
                   &options);
  this->CheckUnary("ascii_rtrim", "[\"BBfooBAB\", null, \"barBAB\", \"BABfooBABbarA\"]",
                   this->type(), "[\"BBfoo\", null, \"bar\", \"BABfooBABbar\"]",
                   &options);
}

#ifdef ARROW_WITH_UTF8PROC
TEST(TestStringKernels, UnicodeLibraryAssumptions) {
  uint8_t output[4];
  for (utf8proc_int32_t codepoint = 0x100; codepoint < 0x110000; codepoint++) {
    utf8proc_ssize_t encoded_nbytes = utf8proc_encode_char(codepoint, output);
    utf8proc_int32_t codepoint_upper = utf8proc_toupper(codepoint);
    utf8proc_ssize_t encoded_nbytes_upper = utf8proc_encode_char(codepoint_upper, output);
    // validate that upper casing will only lead to a byte length growth of max 3/2
    if (encoded_nbytes == 2) {
      EXPECT_LE(encoded_nbytes_upper, 3)
          << "Expected the upper case codepoint for a 2 byte encoded codepoint to be "
             "encoded in maximum 3 bytes, not "
          << encoded_nbytes_upper;
    }
    utf8proc_int32_t codepoint_lower = utf8proc_tolower(codepoint);
    utf8proc_ssize_t encoded_nbytes_lower = utf8proc_encode_char(codepoint_lower, output);
    // validate that lower casing will only lead to a byte length growth of max 3/2
    if (encoded_nbytes == 2) {
      EXPECT_LE(encoded_nbytes_lower, 3)
          << "Expected the lower case codepoint for a 2 byte encoded codepoint to be "
             "encoded in maximum 3 bytes, not "
          << encoded_nbytes_lower;
    }
  }
}
#endif

}  // namespace compute
}  // namespace arrow
