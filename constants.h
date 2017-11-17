// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_CONSTANTS_H_
#define _BSDIFF_CONSTANTS_H_

#include <stdint.h>

namespace bsdiff {

enum class CompressorType {
  kBrotli,
  kBZ2,
};

// The header of the upstream's "BSDIFF40" format using BZ2 as compressor.
constexpr uint8_t kMagicHeader[] = "BSDIFF40";

}  // namespace bsdiff


#endif  // _BSDIFF_CONSTANTS_H_