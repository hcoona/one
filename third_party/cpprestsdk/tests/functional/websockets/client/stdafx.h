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
#include <winsock2.h>
#endif

#include "third_party/cpprestsdk/include/cpprest/asyncrt_utils.h"
#include "third_party/cpprestsdk/include/cpprest/containerstream.h"
#include "third_party/cpprestsdk/include/cpprest/filestream.h"
#include "third_party/cpprestsdk/include/cpprest/producerconsumerstream.h"
#include "third_party/cpprestsdk/include/cpprest/rawptrstream.h"
#include "third_party/cpprestsdk/include/cpprest/ws_client.h"
#include "third_party/cpprestsdk/include/cpprest/ws_msg.h"
#include "os_utilities.h"
#include "test_websocket_server.h"
#include "unittestpp.h"
#include "websocket_client_tests.h"
#include <chrono>
#include <thread>
