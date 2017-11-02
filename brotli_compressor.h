// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_BROTLI_COMPRESSOR_H_
#define _BSDIFF_BROTLI_COMPRESSOR_H_

#include <stdint.h>

#include <vector>

#include <brotli/encode.h>

#include "bsdiff/compressor_buffer.h"
#include "bsdiff/compressor_interface.h"

namespace bsdiff {

class BrotliCompressor : public CompressorInterface {
 public:
  BrotliCompressor();
  ~BrotliCompressor() override;

  // CompressorInterface overrides.
  bool Write(const uint8_t* buf, size_t size) override;
  bool Finish() override;
  const std::vector<uint8_t>& GetCompressedData() override;

 private:
  BrotliEncoderState* brotli_encoder_state_;

  CompressorBuffer comp_buffer_;
};

}  // namespace bsdiff

#endif  // _BSDIFF_BROTLI_COMPRESSOR_H_
