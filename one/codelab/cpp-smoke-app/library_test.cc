#include "one/codelab/cpp-smoke-app/library.h"

#include "gtest/gtest.h"

namespace codelab {
namespace cpp_smoke_app {
namespace {

TEST(LibraryTest, GetAnswer) { EXPECT_EQ(42, GetAnswer()); }

}  // namespace
}  // namespace cpp_smoke_app
}  // namespace codelab
