/***
 * Copyright (C) Microsoft. All rights reserved.
 * Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * stdafx.h
 *
 * Pre-compiled headers
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

#ifndef __PREFIX_H
#define __PREFIX_H

#include <fstream>
#include <memory>
#include <stdio.h>
#include <time.h>
#include <vector>

#if defined(_MSC_VER) && (_MSC_VER >= 1800)
#include <ppltasks.h>
namespace pplx = Concurrency;
#else
#include "third_party/cpprestsdk/include/pplx/pplxtasks.h"
#endif

#include "third_party/cpprestsdk/include/cpprest/asyncrt_utils.h"
#include "third_party/cpprestsdk/include/cpprest/containerstream.h"
#include "third_party/cpprestsdk/include/cpprest/filestream.h"
#include "third_party/cpprestsdk/include/cpprest/interopstream.h"
#include "third_party/cpprestsdk/include/cpprest/producerconsumerstream.h"
#include "third_party/cpprestsdk/include/cpprest/rawptrstream.h"
#include "third_party/cpprestsdk/include/cpprest/streams.h"
#include "streams_tests.h"
#include "unittestpp.h"

template class concurrency::streams::file_buffer<char>;
template class concurrency::streams::file_buffer<wchar_t>;
template class concurrency::streams::streambuf<char>;
template class concurrency::streams::streambuf<wchar_t>;

template class concurrency::streams::rawptr_buffer<char>;
template class concurrency::streams::rawptr_buffer<wchar_t>;
template class concurrency::streams::rawptr_buffer<uint8_t>;
template class concurrency::streams::rawptr_buffer<utf16char>;

template class concurrency::streams::container_buffer<std::vector<uint8_t>>;
template class concurrency::streams::container_buffer<std::vector<char>>;
template class concurrency::streams::container_buffer<std::vector<utf16char>>;

template class concurrency::streams::producer_consumer_buffer<char>;
template class concurrency::streams::producer_consumer_buffer<uint8_t>;
template class concurrency::streams::producer_consumer_buffer<utf16char>;

template class concurrency::streams::container_stream<std::basic_string<char>>;
template class concurrency::streams::container_stream<std::basic_string<wchar_t>>;

#endif
