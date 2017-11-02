// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/brotli_compressor.h"

#include <gtest/gtest.h>

namespace {

const uint8_t kHelloWorld[] = {
    0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x0a,
};
}  // namespace

namespace bsdiff {

TEST(BrotliCompressorTest, BrotliCompressorSmoke) {
  BrotliCompressor brotli_compressor;
  EXPECT_TRUE(brotli_compressor.Write(kHelloWorld, sizeof(kHelloWorld)));
  EXPECT_TRUE(brotli_compressor.Finish());
  std::vector<uint8_t> compressed_data = brotli_compressor.GetCompressedData();
  EXPECT_GT(compressed_data.size(), static_cast<size_t>(0));

  // TODO(xunchang) run brotli decompressor and check we can get back
  // kHelloWorld.
}

}  // namespace bsdiff