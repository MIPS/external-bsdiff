// Copyright 2015 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_BSDIFF_H_
#define _BSDIFF_BSDIFF_H_

#include <stdint.h>

#include "bsdiff/common.h"
#include "bsdiff/patch_writer_interface.h"
#include "bsdiff/suffix_array_index_interface.h"

namespace bsdiff {

// Generate bsdiff patch from the |old_filename| file to the |new_filename|
// file storing the resulting patch in a new |patch_filename| file.
BSDIFF_EXPORT
int bsdiff(const char* old_filename,
           const char* new_filename,
           const char* patch_filename);

// Generate bsdiff patch from |old_buf| to |new_buf|, save the patch file to
// |patch_filename|. Returns 0 on success.
// |I_cache| can be used to cache the suffix array if the same |old_buf| is used
// repeatedly, pass nullptr if not needed.
BSDIFF_EXPORT
int bsdiff(const uint8_t* old_buf,
           size_t oldsize,
           const uint8_t* new_buf,
           size_t newsize,
           const char* patch_filename,
           SuffixArrayIndexInterface** sai_cache);

BSDIFF_EXPORT
int bsdiff(const uint8_t* old_buf,
           size_t oldsize,
           const uint8_t* new_buf,
           size_t newsize,
           PatchWriterInterface* patch,
           SuffixArrayIndexInterface** sai_cache);

}  // namespace bsdiff

#endif  // _BSDIFF_BSDIFF_H_
