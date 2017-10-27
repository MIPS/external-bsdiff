// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_PATCH_WRITER_FACTORY_H_
#define _BSDIFF_PATCH_WRITER_FACTORY_H_

#include <memory>
#include <string>

#include "bsdiff/patch_writer_interface.h"

namespace bsdiff {

// Create a patch writer compatible with upstream's "BSDIFF40" patch format
// using bz2 as a compressor.
std::unique_ptr<PatchWriterInterface> CreateBsdiffPatchWriter(
    const std::string& patch_filename);

}  // namespace bsdiff

#endif  // _BSDIFF_PATCH_WRITER_FACTORY_H_
