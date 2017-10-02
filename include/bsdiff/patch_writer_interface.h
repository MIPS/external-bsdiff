// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_PATCH_WRITER_INTERFACE_H_
#define _BSDIFF_PATCH_WRITER_INTERFACE_H_

#include <stdint.h>

#include <cstddef>

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

class PatchWriterInterface {
 public:
  virtual ~PatchWriterInterface() = default;

  // Initialize the patch writer.
  virtual bool Init() = 0;

  // Write the passed |data| buffer of length |size| to the Diff or Extra
  // streams respectively. Each method can be called independently from each
  // other and calls don't need to be a correlation with the AddControlEntry()
  // until Close() is called.
  virtual bool WriteDiffStream(const uint8_t* data, size_t size) = 0;
  virtual bool WriteExtraStream(const uint8_t* data, size_t size) = 0;

  // Add a new control triplet entry to the patch. These triplets may be added
  // at any point before calling Close(), regardless of whether the
  // corresponding WriteDiffStream() and WriteExtraStream() have been called
  // yet.
  virtual bool AddControlEntry(const ControlEntry& entry) = 0;

  // Finalize the patch writing process and close the file.
  virtual bool Close() = 0;

 protected:
  PatchWriterInterface() = default;
};

}  // namespace bsdiff

#endif  // _BSDIFF_PATCH_WRITER_INTERFACE_H_
