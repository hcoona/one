/***
 * Copyright (C) Microsoft. All rights reserved.
 * Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
 *
 * =+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *
 * Pre-compiled headers
 *
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 ****/

#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include "third_party/cpprestsdk/include/pplx/pplxtasks.h"
#include <fstream>
#include <memory>
#include <stdio.h>
#include <time.h>
#include <vector>

#if defined(_WIN32)
#include "third_party/cpprestsdk/include/pplx/pplxconv.h"
#else
#include "third_party/cpprestsdk/include/pplx/threadpool.h"
#endif

#include "third_party/cpprestsdk/include/cpprest/asyncrt_utils.h"
#include "os_utilities.h"
#include "unittestpp.h"
