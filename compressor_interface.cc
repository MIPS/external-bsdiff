// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/compressor_interface.h"

#include "bsdiff/brotli_compressor.h"
#include "bsdiff/bz2_compressor.h"
#include "bsdiff/logging.h"

namespace bsdiff {

std::unique_ptr<CompressorInterface> CreateCompressor(CompressorType type) {
  switch (type) {
    case CompressorType::kBZ2:
      return std::unique_ptr<CompressorInterface>(new BZ2Compressor());
    case CompressorType::kBrotli:
      return std::unique_ptr<CompressorInterface>(new BrotliCompressor());
    default:
      LOG(ERROR) << "unsupported compressor type: "
                 << static_cast<uint8_t>(type) << std::endl;
      return nullptr;
  }
}

}  // namespace bsdiff
