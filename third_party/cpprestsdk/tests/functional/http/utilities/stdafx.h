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
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>

#include <Windows.h>
#endif

#include "third_party/cpprestsdk/include/cpprest/asyncrt_utils.h"
#include "third_party/cpprestsdk/include/cpprest/http_client.h"
#include "third_party/cpprestsdk/include/cpprest/http_msg.h"
#include "third_party/cpprestsdk/include/cpprest/uri.h"
#include "include/http_asserts.h"
#include "unittestpp.h"
