// Tencent is pleased to support the open source community by making RapidJSON available.
//
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#ifndef PERFTEST_H_
#define PERFTEST_H_

#include <stdlib.h>

#include "tools/cpp/runfiles/runfiles.h"

#define TEST_RAPIDJSON  1
#define TEST_PLATFORM   0
#define TEST_MISC       0

#define TEST_VERSION_CODE(x,y,z) \
  (((x)*100000) + ((y)*100) + (z))

// __SSE2__ and __SSE4_2__ are recognized by gcc, clang, and the Intel compiler.
// We use -march=native with gmake to enable -msse2 and -msse4.2, if supported.
// Likewise, __ARM_NEON is used to detect Neon.
#if defined(__SSE4_2__) && !defined(RAPIDJSON_SSE42)
#  define RAPIDJSON_SSE42
#elif defined(__SSE2__) && !defined(RAPIDJSON_SSE2)
#  define RAPIDJSON_SSE2
#elif defined(__ARM_NEON) && !defined(RAPIDJSON_NEON)
#  define RAPIDJSON_NEON
#endif

#define RAPIDJSON_HAS_STDSTRING 1

////////////////////////////////////////////////////////////////////////////////
// Google Test

#ifdef __cplusplus

// gtest indirectly included inttypes.h, without __STDC_CONSTANT_MACROS.
#ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS 1 // required by C++ standard
#endif

#if defined(__clang__) || defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2))
#if defined(__clang__) || (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic push
#endif
#pragma GCC diagnostic ignored "-Weffc++"
#endif

#include "gtest/gtest.h"

#if defined(__clang__) || defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#pragma warning(disable : 4996) // 'function': was declared deprecated
#endif

namespace details {

inline size_t fixAlign16(size_t v) {
  static constexpr size_t kAlignment = 16U;
  return (v + kAlignment - 1) & ~(kAlignment - 1);
}

}  // namespace details

//! Base class for all performance tests
class PerfTest : public ::testing::Test {
public:
    PerfTest() : filename_(), json_(), length_(), whitespace_(), whitespace_length_() {}

    virtual void SetUp() {
        {
            using bazel::tools::cpp::runfiles::Runfiles;
            std::string error;
            std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
            ASSERT_TRUE(runfiles) << error;
            static constexpr char kPath[] =
                "com_github_hcoona_one/third_party/rapidjson/bin/data/"
                "sample.json";

            filename_storage_ = runfiles->Rlocation(kPath);
            FILE *fp = fopen(filename_ = filename_storage_.c_str(), "rb");
            ASSERT_TRUE(fp != 0);

            fseek(fp, 0, SEEK_END);
            length_ = (size_t)ftell(fp);
            fseek(fp, 0, SEEK_SET);
            json_ = (char*)malloc(details::fixAlign16(length_ + 1));
            ASSERT_EQ(length_, fread(json_, 1, length_, fp));
            json_[length_] = '\0';
            fclose(fp);
        }

        // whitespace test
        {
            whitespace_length_ = 1024 * 1024;
            whitespace_ = (char *)malloc(details::fixAlign16(whitespace_length_  + 4));
            char *p = whitespace_;
            for (size_t i = 0; i < whitespace_length_; i += 4) {
                *p++ = ' ';
                *p++ = '\n';
                *p++ = '\r';
                *p++ = '\t';
            }
            *p++ = '[';
            *p++ = '0';
            *p++ = ']';
            *p++ = '\0';
        }

        // types test
        {
            using bazel::tools::cpp::runfiles::Runfiles;
            std::string error;
            std::unique_ptr<Runfiles> runfiles(Runfiles::CreateForTest(&error));
            ASSERT_TRUE(runfiles) << error;
            static constexpr char kPathPrefix[] =
                "com_github_hcoona_one/third_party/rapidjson/bin/types/";

            const char* typesfilenames[] = {
                "booleans.json",
                "floats.json",
                "guids.json",
                "integers.json",
                "mixed.json",
                "nulls.json",
                "paragraphs.json",
                "alotofkeys.json"
            };

            for (size_t j = 0; j < sizeof(typesfilenames) / sizeof(typesfilenames[0]); j++) {
                types_[j] = 0;
                std::string filename = std::string(kPathPrefix) + typesfilenames[j];
                std::string rfilename = runfiles->Rlocation(filename.c_str());
                FILE* fp = fopen(rfilename.c_str(), "rb");
                ASSERT_TRUE(fp != 0) << rfilename;
                fseek(fp, 0, SEEK_END);
                typesLength_[j] = (size_t)ftell(fp);
                fseek(fp, 0, SEEK_SET);
                types_[j] = (char*)malloc(details::fixAlign16(typesLength_[j] + 1));
                ASSERT_EQ(typesLength_[j], fread(types_[j], 1, typesLength_[j], fp));
                types_[j][typesLength_[j]] = '\0';
                fclose(fp);
            }
        }
    }

    virtual void TearDown() {
        free(json_);
        free(whitespace_);
        json_ = 0;
        whitespace_ = 0;
        for (size_t i = 0; i < 8; i++) {
            free(types_[i]);
            types_[i] = 0;
        }
    }

private:
    PerfTest(const PerfTest&);
    PerfTest& operator=(const PerfTest&);

protected:
    const char* filename_;
    std::string filename_storage_;
    char *json_;
    size_t length_;
    char *whitespace_;
    size_t whitespace_length_;
    char *types_[8];
    size_t typesLength_[8];

    static const size_t kTrialCount = 1000;
};

#endif // __cplusplus

#endif // PERFTEST_H_
