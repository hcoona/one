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

#pragma once

#include <fstream>
#include <memory>
#include <stdio.h>
#include <time.h>
#include <vector>

#if (defined(_MSC_VER) && (_MSC_VER >= 1800)) && !CPPREST_FORCE_PPLX
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
#include "os_utilities.h"
#include "streams_tests.h"
#include "unittestpp.h"
