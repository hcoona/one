# HCOONa's ONE repo

ONE: the mono repo for me.

See https://monorepo.tools for "What is a monorepo" & "Why a monorepo". And [Why Google Stores Billions of Lines of Code in a Single Repository](https://cacm.acm.org/magazines/2016/7/204032-why-google-stores-billions-of-lines-of-code-in-a-single-repository/fulltext) is also a good reading material.

## Projects

* [base](base/index.md): fundamental utilities. (conceptually like Google abseil-cpp & Facebook folly)
* [codelab](codelab/index.md): experimental codes.
* [jinduo（金铎）](jinduo/index.md): port of muduo（木铎） library (not started yet).

## Future

Build all external projects with bazel native way instead of foreign_cc rules. Because foreign_cc rule would prevent tracking the source files during code navigation. And leverage little bazel cache facilities to reduce build time.
