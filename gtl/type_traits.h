#ifndef GTL_TYPE_TRAITS_H_
#define GTL_TYPE_TRAITS_H_

#include <type_traits>

#include "absl/meta/type_traits.h"

// Some versions of libstdc++ have partial support for type_traits, but misses
// a smaller subset while removing some of the older non-standard stuff. Assume
// that all versions below 5.0 fall in this category, along with one 5.0
// experimental release. Test for this by consulting compiler major version,
// the only reliable option available, so theoretically this could fail should
// you attempt to mix an earlier version of libstdc++ with >= GCC5. But
// that's unlikely to work out, especially as GCC5 changed ABI.
#define CR_GLIBCXX_5_0_0 20150123
#if (defined(__GNUC__) && __GNUC__ < 5) || \
    (defined(__GLIBCXX__) && __GLIBCXX__ == CR_GLIBCXX_5_0_0)
#define CR_USE_FALLBACKS_FOR_OLD_EXPERIMENTAL_GLIBCXX
#endif

// This hacks around using gcc with libc++ which has some incompatibilies.
// - is_trivially_* doesn't work: https://llvm.org/bugs/show_bug.cgi?id=27538
// TODO(danakj): Remove this when android builders are all using a newer version
// of gcc, or the android ndk is updated to a newer libc++ that works with older
// gcc versions.
#if !defined(__clang__) && defined(_LIBCPP_VERSION)
#define CR_USE_FALLBACKS_FOR_GCC_WITH_LIBCXX
#endif

namespace gtl {

namespace internal {

// From base/stl_util.h

// From base/template_util.h

// Used to detech whether the given type is an iterator.  This is normally used
// with std::enable_if to provide disambiguation for functions that take
// templatzed iterators as input.
template <typename T, typename = void>
struct is_iterator : std::false_type {};

template <typename T>
struct is_iterator<
    T, absl::void_t<typename std::iterator_traits<T>::iterator_category>>
    : std::true_type {};

}  // namespace internal

// From base/stl_util.h

// C++14 implementation of C++17's std::as_const():
// https://en.cppreference.com/w/cpp/utility/as_const
template <typename T>
constexpr std::add_const_t<T>& as_const(T& t) noexcept {  // NOLINT
  return t;
}

template <typename T>
void as_const(const T&& t) = delete;

// From base/template_util.h

// is_trivially_copyable is especially hard to get right.
// - Older versions of libstdc++ will fail to have it like they do for other
//   type traits. This has become a subset of the second point, but used to be
//   handled independently.
// - An experimental release of gcc includes most of type_traits but misses
//   is_trivially_copyable, so we still have to avoid using libstdc++ in this
//   case, which is covered by CR_USE_FALLBACKS_FOR_OLD_EXPERIMENTAL_GLIBCXX.
// - When compiling libc++ from before r239653, with a gcc compiler, the
//   std::is_trivially_copyable can fail. So we need to work around that by not
//   using the one in libc++ in this case. This is covered by the
//   CR_USE_FALLBACKS_FOR_GCC_WITH_LIBCXX define, and is discussed in
//   https://llvm.org/bugs/show_bug.cgi?id=27538#c1 where they point out that
//   in libc++'s commit r239653 this is fixed by libc++ checking for gcc 5.1.
// - In both of the above cases we are using the gcc compiler. When defining
//   this ourselves on compiler intrinsics, the __is_trivially_copyable()
//   intrinsic is not available on gcc before version 5.1 (see the discussion in
//   https://llvm.org/bugs/show_bug.cgi?id=27538#c1 again), so we must check for
//   that version.
// - When __is_trivially_copyable() is not available because we are on gcc older
//   than 5.1, we need to fall back to something, so we use __has_trivial_copy()
//   instead based on what was done one-off in bit_cast() previously.

// TODO(crbug.com/554293): Remove this when all platforms have this in the std
// namespace and it works with gcc as needed.
#if defined(CR_USE_FALLBACKS_FOR_OLD_EXPERIMENTAL_GLIBCXX) || \
    defined(CR_USE_FALLBACKS_FOR_GCC_WITH_LIBCXX)
template <typename T>
struct is_trivially_copyable {
// TODO(danakj): Remove this when android builders are all using a newer version
// of gcc, or the android ndk is updated to a newer libc++ that does this for
// us.
#if _GNUC_VER >= 501
  static constexpr bool value = __is_trivially_copyable(T);
#else
  static constexpr bool value =
      __has_trivial_copy(T) && __has_trivial_destructor(T);
#endif
};
#else
template <class T>
using is_trivially_copyable = std::is_trivially_copyable<T>;
#endif

}  // namespace gtl

#endif  // GTL_TYPE_TRAITS_H_
