// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/decompressor_interface.h"

#include "bsdiff/brotli_decompressor.h"
#include "bsdiff/bz2_decompressor.h"

namespace bsdiff {

std::unique_ptr<DecompressorInterface> CreateDecompressor(CompressorType type) {
  if (type == CompressorType::kBrotli) {
    return std::unique_ptr<DecompressorInterface>(new BrotliDecompressor());
  }
  // Use BZ2 as a default decompressor.
  return std::unique_ptr<DecompressorInterface>(new BZ2Decompressor());
}

}  // namespace bsdiff