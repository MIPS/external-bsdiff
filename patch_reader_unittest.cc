// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unistd.h>

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "bsdiff/bz2_compressor.h"
#include "bsdiff/patch_reader.h"
#include "bsdiff/utils.h"

namespace {

void EncodeInt64(int64_t x, uint8_t* buf) {
  uint64_t y = x < 0 ? (1ULL << 63ULL) - x : x;
  for (int i = 0; i < 8; ++i) {
    buf[i] = y & 0xff;
    y /= 256;
  }
}

}  // namespace

namespace bsdiff {

TEST(PatchReaderTest, BZ2PatchReaderSmoke) {
  BZ2Compressor ctrl_stream;
  BZ2Compressor diff_stream;
  BZ2Compressor extra_stream;

  // Write 3 blocks of data to diff stream & extra stream respectively.
  std::vector<std::string> diff_data = {"HelloWorld", "BspatchPatchTest",
                                        "BspatchDiffData"};
  std::vector<std::string> extra_data = {"HelloWorld!", "BZ2PatchReaderSmoke",
                                         "BspatchExtraData"};
  std::vector<int64_t> offset_increment = {100, 200, 300};
  for (size_t i = 0; i < diff_data.size(); i++) {
    uint8_t buf[24];
    EncodeInt64(diff_data[i].size(), buf);
    EncodeInt64(extra_data[i].size(), buf + 8);
    EncodeInt64(offset_increment[i], buf + 16);
    EXPECT_TRUE(ctrl_stream.Write(buf, sizeof(buf)));
    EXPECT_TRUE(
        diff_stream.Write(reinterpret_cast<const uint8_t*>(diff_data[i].data()),
                          diff_data[i].size()));
    EXPECT_TRUE(extra_stream.Write(
        reinterpret_cast<const uint8_t*>(extra_data[i].data()),
        extra_data[i].size()));
  }
  EXPECT_TRUE(ctrl_stream.Finish());
  EXPECT_TRUE(diff_stream.Finish());
  EXPECT_TRUE(extra_stream.Finish());

  // Encode the header
  uint8_t buf[24];
  EncodeInt64(ctrl_stream.GetCompressedData().size(), buf);
  EncodeInt64(diff_stream.GetCompressedData().size(), buf + 8);
  EncodeInt64(500, buf + 16);

  std::vector<uint8_t> patch_data;
  std::copy(kMagicHeader, kMagicHeader + 8, std::back_inserter(patch_data));
  std::copy(buf, buf + sizeof(buf), std::back_inserter(patch_data));

  // Concatenate the three streams into one patch.
  std::copy(ctrl_stream.GetCompressedData().begin(),
            ctrl_stream.GetCompressedData().end(),
            std::back_inserter(patch_data));
  std::copy(diff_stream.GetCompressedData().begin(),
            diff_stream.GetCompressedData().end(),
            std::back_inserter(patch_data));
  std::copy(extra_stream.GetCompressedData().begin(),
            extra_stream.GetCompressedData().end(),
            std::back_inserter(patch_data));

  BsdiffPatchReader patch_reader;
  EXPECT_TRUE(patch_reader.Init(patch_data.data(), patch_data.size()));
  EXPECT_EQ(500ull, patch_reader.new_file_size());
  // Check that the decompressed data matches what we wrote.
  for (size_t i = 0; i < diff_data.size(); i++) {
    ControlEntry control_entry(0, 0, 0);
    EXPECT_TRUE(patch_reader.ParseControlEntry(&control_entry));
    uint8_t buffer[128] = {};
    EXPECT_TRUE(patch_reader.ReadDiffStream(buffer, diff_data[i].size()));
    EXPECT_EQ(0, memcmp(buffer, diff_data[i].data(), diff_data[i].size()));
    EXPECT_TRUE(patch_reader.ReadExtraStream(buffer, extra_data[i].size()));
    EXPECT_EQ(0, memcmp(buffer, extra_data[i].data(), extra_data[i].size()));
  }
  EXPECT_TRUE(patch_reader.Finish());
}

}  // namespace bsdiff
