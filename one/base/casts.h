#pragma once

#include <memory>
#include <utility>

#include "absl/debugging/internal/demangle.h"
#include "absl/strings/string_view.h"
#include "glog/logging.h"

// Copied from
// https://github.com/protocolbuffers/protobuf/blob/v3.12.3/src/google/protobuf/stubs/casts.h
// Modified:
// 1. Removed bit_cast because we use abseil one.
// 2. Put implicit_cast into internal namespace to avoid conflicts with abseil
// one.

// Protocol Buffers - Google's data interchange format
// Copyright 2014 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#if !defined(NDEBUG)
constexpr size_t kDemangleNameBufferSize = 1024;
#endif  // !defined(NDEBUG)

namespace hcoona {

namespace internal {

// Use implicit_cast as a safe version of static_cast or const_cast
// for upcasting in the type hierarchy (i.e. casting a pointer to Foo
// to a pointer to SuperclassOfFoo or casting a pointer to Foo to
// a const pointer to Foo).
// When you use implicit_cast, the compiler checks that the cast is safe.
// Such explicit implicit_casts are necessary in surprisingly many
// situations where C++ demands an exact type match instead of an
// argument type convertable to a target type.
//
// The From type can be inferred, so the preferred syntax for using
// implicit_cast is the same as for static_cast etc.:
//
//   implicit_cast<ToType>(expr)
//
// implicit_cast would have been part of the C++ standard library,
// but the proposal was submitted too late.  It will probably make
// its way into the language in the future.
template <typename To, typename From>
inline To implicit_cast(From const& f) {
  return f;
}

}  // namespace internal

// When you upcast (that is, cast a pointer from type Foo to type
// SuperclassOfFoo), it's fine to use implicit_cast<>, since upcasts
// always succeed.  When you downcast (that is, cast a pointer from
// type Foo to type SubclassOfFoo), static_cast<> isn't safe, because
// how do you know the pointer is really of type SubclassOfFoo?  It
// could be a bare Foo, or of type DifferentSubclassOfFoo.  Thus,
// when you downcast, you should use this macro.  In debug mode, we
// use dynamic_cast<> to double-check the downcast is legal (we die
// if it's not).  In normal mode, we do the efficient static_cast<>
// instead.  Thus, it's important to test in debug mode to make sure
// the cast is legal!
//    This is the only place in the code we should use dynamic_cast<>.
// In particular, you SHOULDN'T be using dynamic_cast<> in order to
// do RTTI (eg code like this:
//    if (dynamic_cast<Subclass1>(foo)) HandleASubclass1Object(foo);
//    if (dynamic_cast<Subclass2>(foo)) HandleASubclass2Object(foo);
// You should design the code some other way not to need this.

template <typename To, typename From>  // use like this: down_cast<T*>(foo);
inline To down_cast(From* f) {         // so we only accept pointers
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    internal::implicit_cast<From*, To>(0);
  }

#if !defined(NDEBUG)
  // RTTI: debug mode only!
  absl::string_view from_type_name;
  char from_type_name_buffer[kDemangleNameBufferSize];
  if (absl::debugging_internal::Demangle(
          typeid(*f).name(), from_type_name_buffer, kDemangleNameBufferSize)) {
    from_type_name = from_type_name_buffer;
  } else {
    from_type_name = typeid(*f).name();
  }

  absl::string_view to_type_name;
  char to_type_name_buffer[kDemangleNameBufferSize];
  if (absl::debugging_internal::Demangle(typeid(To).name(), to_type_name_buffer,
                                         kDemangleNameBufferSize)) {
    to_type_name = to_type_name_buffer;
  } else {
    to_type_name = typeid(To).name();
  }

  DCHECK(f == nullptr || dynamic_cast<To>(f) != nullptr)
      << "Failed to cast given pointer \"" << reinterpret_cast<void*>(f)
      << "\" from type \"" << from_type_name << "\" to type \"" << to_type_name
      << "\"";
#endif
  return static_cast<To>(f);
}

template <typename To, typename From>  // use like this: down_cast<T&>(foo);
inline To down_cast(From& f) {         // NOLINT
  typedef typename std::remove_reference<To>::type* ToAsPointer;
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    internal::implicit_cast<From*, ToAsPointer>(0);
  }

#if !defined(NDEBUG)
  // RTTI: debug mode only!
  absl::string_view from_type_name;
  char from_type_name_buffer[kDemangleNameBufferSize];
  if (absl::debugging_internal::Demangle(
          typeid(f).name(), from_type_name_buffer, kDemangleNameBufferSize)) {
    from_type_name = from_type_name_buffer;
  } else {
    from_type_name = typeid(f).name();
  }

  absl::string_view to_type_name;
  char to_type_name_buffer[kDemangleNameBufferSize];
  if (absl::debugging_internal::Demangle(typeid(To).name(), to_type_name_buffer,
                                         kDemangleNameBufferSize)) {
    to_type_name = to_type_name_buffer;
  } else {
    to_type_name = typeid(To).name();
  }

  DCHECK(dynamic_cast<ToAsPointer>(&f) != nullptr)
      << "Failed to cast given reference \""
      << reinterpret_cast<const void*>(&f) << "\" from type \""
      << from_type_name << "\" to type \"" << to_type_name << "\"";
#endif
  return *static_cast<ToAsPointer>(&f);
}

template <typename To,
          typename From>  // use like this: down_pointer_cast<To>(foo);
inline std::shared_ptr<To> down_pointer_cast(std::shared_ptr<From> f) noexcept {
  typedef typename std::remove_reference<To>::type* ToAsPointer;
  // Ensures that To is a sub-type of From *.  This test is here only
  // for compile-time type checking, and has no overhead in an
  // optimized build at run-time, as it will be optimized away
  // completely.
  if (false) {
    internal::implicit_cast<From*, ToAsPointer>(0);
  }

#if !defined(NDEBUG)
  // RTTI: debug mode only!
  absl::string_view from_type_name;
  char from_type_name_buffer[kDemangleNameBufferSize];
  if (absl::debugging_internal::Demangle(
          typeid(*f).name(), from_type_name_buffer, kDemangleNameBufferSize)) {
    from_type_name = from_type_name_buffer;
  } else {
    from_type_name = typeid(*f).name();
  }

  absl::string_view to_type_name;
  char to_type_name_buffer[kDemangleNameBufferSize];
  if (absl::debugging_internal::Demangle(typeid(To).name(), to_type_name_buffer,
                                         kDemangleNameBufferSize)) {
    to_type_name = to_type_name_buffer;
  } else {
    to_type_name = typeid(To).name();
  }

  DCHECK(f == nullptr || std::dynamic_pointer_cast<To>(f) != nullptr)
      << "Failed to cast given pointer \"" << reinterpret_cast<void*>(f.get())
      << "\" from type \"" << from_type_name << "\" to type \"" << to_type_name
      << "\"";
#endif

  return std::static_pointer_cast<To>(std::move(f));
}

}  // namespace hcoona
