// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "gtl/test/copy_only_int.h"

namespace gtl {

// static
int CopyOnlyInt::num_copies_ = 0;

}  // namespace gtl
