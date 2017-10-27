// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <bsdiff/compressor_interface.h>

#include <bsdiff/brotli_compressor.h>
#include <bsdiff/bz2_compressor.h>

namespace bsdiff {

std::unique_ptr<CompressorInterface> CreateCompressor(CompressorType type) {
  if (type == CompressorType::kBrotli) {
    return std::unique_ptr<CompressorInterface>(new BrotliCompressor());
  }
  // Use BZ2 as a default compressor.
  return std::unique_ptr<CompressorInterface>(new BZ2Compressor());
}

}  // namespace bsdiff
