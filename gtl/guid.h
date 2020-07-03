// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GTL_GUID_H_
#define GTL_GUID_H_

#include <stdint.h>

#include <string>

#include "absl/strings/string_view.h"

namespace gtl {

// Generate a 128-bit random GUID in the form of version 4 as described in
// RFC 4122, section 4.4.
// The format of GUID version 4 must be xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx,
// where y is one of [8, 9, A, B].
// The hexadecimal values "a" through "f" are output as lower case characters.
//
// A cryptographically secure random source will be used, but consider using
// UnguessableToken for greater type-safety if GUID format is unnecessary.
std::string GenerateGUID();

// Returns true if the input string conforms to the version 4 GUID format.
// Note that this does NOT check if the hexadecimal values "a" through "f"
// are in lower case characters, as Version 4 RFC says onput they're
// case insensitive. (Use IsValidGUIDOutputString for checking if the
// given string is valid output string)
bool IsValidGUID(absl::string_view guid);
#ifdef ABSL_USES_STD_STRING_VIEW
bool IsValidGUID(std::u16string_view guid);
#endif  // ABSL_USES_STD_STRING_VIEW

// Returns true if the input string is valid version 4 GUID output string.
// This also checks if the hexadecimal values "a" through "f" are in lower
// case characters.
bool IsValidGUIDOutputString(absl::string_view guid);

// For unit testing purposes only.  Do not use outside of tests.
std::string RandomDataToGUIDString(const uint64_t bytes[2]);

}  // namespace gtl

#endif  // GTL_GUID_H_
