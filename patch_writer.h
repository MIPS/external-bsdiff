// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_PATCH_WRITER_H_
#define _BSDIFF_PATCH_WRITER_H_

#include <stdio.h>

#include <string>
#include <vector>

#include "bsdiff/common.h"
#include "bz2_compressor.h"

namespace bsdiff {

struct ControlEntry {
  ControlEntry(uint64_t diff_size,
               uint64_t extra_size,
               int64_t offset_increment)
      : diff_size(diff_size),
        extra_size(extra_size),
        offset_increment(offset_increment) {}

  // The number of bytes to copy from the source and diff stream.
  uint64_t diff_size;

  // The number of bytes to copy from the extra stream.
  uint64_t extra_size;

  // The value to add to the source pointer after patching from the diff stream.
  int64_t offset_increment;
};

class BsdiffPatchWriter {
 public:
  BsdiffPatchWriter(const uint8_t* old_buf,
                    uint64_t old_size,
                    const uint8_t* new_buf,
                    uint64_t new_size)
      : old_buf_(old_buf),
        old_size_(old_size),
        new_buf_(new_buf),
        new_size_(new_size) {}

  // Create the file |patch_filename| where the patch will be written to.
  bool Open(const std::string& patch_filename);

  // Add a new control triplet entry to the patch. The |entry.diff_size| bytes
  // for the diff stream and the |entry.extra_size| bytes for the extra stream
  // will be computed and added to the corresponding streams in the patch.
  // Returns whether the operation succeeded. The operation can fail if either
  // the old or new files are referenced out of bounds.
  bool AddControlEntry(const ControlEntry& entry);

  bool Close();

 private:
  // Write the BSDIFF patch header to the |fp_| given the size of the compressed
  // control block and the compressed diff block.
  bool WriteHeader(uint64_t ctrl_size, uint64_t diff_size);


  const uint8_t* old_buf_;
  uint64_t old_size_;
  const uint8_t* new_buf_;
  uint64_t new_size_;

  // Bytes of the new_buf_ already written.
  uint64_t written_output_{0};

  // The current position in the old buf.
  int64_t old_pos_{0};

  // The current file we are writing to.
  FILE* fp_{nullptr};

  // The three internal compressed streams.
  BZ2Compressor ctrl_stream_;
  BZ2Compressor diff_stream_;
  BZ2Compressor extra_stream_;
};

}  // namespace bsdiff

#endif  // _BSDIFF_PATCH_WRITER_H_
