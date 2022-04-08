// This file is @generated by then_compile_test.rb. Do not edit directly.

#include "folly/futures/test/ThenCompileTest.h"

using namespace folly;

TEST(Basic, thenVariants) {
  SomeClass anObject;

  { Future<B> f = someFuture<A>().then(&aFunction<Future<B>, Try<A>&&>); }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aStaticMethod<Future<B>, Try<A>&&>);
  }
  { Future<B> f = someFuture<A>().then(aStdFunction<Future<B>, Try<A>&&>()); }
  {
    Future<B> f =
        someFuture<A>().then([&](Try<A>&&) { return someFuture<B>(); });
  }
  { Future<B> f = someFuture<A>().then(&aFunction<Future<B>, Try<A> const&>); }
  {
    Future<B> f = someFuture<A>().then(
        &SomeClass::aStaticMethod<Future<B>, Try<A> const&>);
  }
  {
    Future<B> f =
        someFuture<A>().then(aStdFunction<Future<B>, Try<A> const&>());
  }
  {
    Future<B> f =
        someFuture<A>().then([&](Try<A> const&) { return someFuture<B>(); });
  }
  { Future<B> f = someFuture<A>().then(&aFunction<Future<B>, Try<A>>); }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aStaticMethod<Future<B>, Try<A>>);
  }
  { Future<B> f = someFuture<A>().then(aStdFunction<Future<B>, Try<A>>()); }
  {
    Future<B> f = someFuture<A>().then([&](Try<A>) { return someFuture<B>(); });
  }
  { Future<B> f = someFuture<A>().then(&aFunction<B, Try<A>&&>); }
  {
    Future<B> f = someFuture<A>().then(&SomeClass::aStaticMethod<B, Try<A>&&>);
  }
  { Future<B> f = someFuture<A>().then(aStdFunction<B, Try<A>&&>()); }
  {
    Future<B> f = someFuture<A>().then([&](Try<A>&&) { return B(); });
  }
  { Future<B> f = someFuture<A>().then(&aFunction<B, Try<A> const&>); }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aStaticMethod<B, Try<A> const&>);
  }
  { Future<B> f = someFuture<A>().then(aStdFunction<B, Try<A> const&>()); }
  {
    Future<B> f = someFuture<A>().then([&](Try<A> const&) { return B(); });
  }
  { Future<B> f = someFuture<A>().then(&aFunction<B, Try<A>>); }
  { Future<B> f = someFuture<A>().then(&SomeClass::aStaticMethod<B, Try<A>>); }
  { Future<B> f = someFuture<A>().then(aStdFunction<B, Try<A>>()); }
  {
    Future<B> f = someFuture<A>().then([&](Try<A>) { return B(); });
  }
  { Future<B> f = someFuture<A>().thenValue(&aFunction<Future<B>, A&&>); }
  {
    Future<B> f =
        someFuture<A>().thenValue(&SomeClass::aStaticMethod<Future<B>, A&&>);
  }
  { Future<B> f = someFuture<A>().thenValue(aStdFunction<Future<B>, A&&>()); }
  {
    Future<B> f =
        someFuture<A>().thenValue([&](A&&) { return someFuture<B>(); });
  }
  { Future<B> f = someFuture<A>().thenValue(&aFunction<Future<B>, A const&>); }
  {
    Future<B> f = someFuture<A>().thenValue(
        &SomeClass::aStaticMethod<Future<B>, A const&>);
  }
  {
    Future<B> f =
        someFuture<A>().thenValue(aStdFunction<Future<B>, A const&>());
  }
  {
    Future<B> f =
        someFuture<A>().thenValue([&](A const&) { return someFuture<B>(); });
  }
  { Future<B> f = someFuture<A>().thenValue(&aFunction<Future<B>, A>); }
  {
    Future<B> f =
        someFuture<A>().thenValue(&SomeClass::aStaticMethod<Future<B>, A>);
  }
  { Future<B> f = someFuture<A>().thenValue(aStdFunction<Future<B>, A>()); }
  {
    Future<B> f = someFuture<A>().thenValue([&](A) { return someFuture<B>(); });
  }
  { Future<B> f = someFuture<A>().thenValue(&aFunction<B, A&&>); }
  {
    Future<B> f = someFuture<A>().thenValue(&SomeClass::aStaticMethod<B, A&&>);
  }
  { Future<B> f = someFuture<A>().thenValue(aStdFunction<B, A&&>()); }
  {
    Future<B> f = someFuture<A>().thenValue([&](A&&) { return B(); });
  }
  { Future<B> f = someFuture<A>().thenValue(&aFunction<B, A const&>); }
  {
    Future<B> f =
        someFuture<A>().thenValue(&SomeClass::aStaticMethod<B, A const&>);
  }
  { Future<B> f = someFuture<A>().thenValue(aStdFunction<B, A const&>()); }
  {
    Future<B> f = someFuture<A>().thenValue([&](A const&) { return B(); });
  }
  { Future<B> f = someFuture<A>().thenValue(&aFunction<B, A>); }
  { Future<B> f = someFuture<A>().thenValue(&SomeClass::aStaticMethod<B, A>); }
  { Future<B> f = someFuture<A>().thenValue(aStdFunction<B, A>()); }
  {
    Future<B> f = someFuture<A>().thenValue([&](A) { return B(); });
  }
  {
    Future<B> f = someFuture<A>().then(
        &SomeClass::aMethod<Future<B>, Try<A>&&>, &anObject);
  }
  {
    Future<B> f = someFuture<A>().then(
        &SomeClass::aMethod<Future<B>, Try<A> const&>, &anObject);
  }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aMethod<Future<B>, Try<A>>, &anObject);
  }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aMethod<B, Try<A>&&>, &anObject);
  }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aMethod<B, Try<A> const&>, &anObject);
  }
  {
    Future<B> f =
        someFuture<A>().then(&SomeClass::aMethod<B, Try<A>>, &anObject);
  }
}
