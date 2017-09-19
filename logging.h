// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_LOGGING_H_
#define _BSDIFF_LOGGING_H_

#include <iostream>

// Simple error logging macro to avoid dependencies in other base libraries.
// Unfortunately, you must pass std::endl as the last element on the stream to
// produce the new line.
#define LOG(X) (std::cerr << #X << " " << __FILE__ << ":" << __LINE__ << ": ")

#endif  // _BSDIFF_LOGGING_H_
