// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/patch_writer_factory.h"

#include "bsdiff/patch_writer.h"

namespace bsdiff {

// TODO(xunchang) choose the compression type based on the input parameter.
std::unique_ptr<PatchWriterInterface> CreateBsdiffPatchWriter(
    const std::string& patch_filename) {
  return std::unique_ptr<PatchWriterInterface>(
      new BsdiffPatchWriter(patch_filename, CompressorType::kBZ2));
}

}  // namespace bsdiff
