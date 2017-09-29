// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_PATCH_WRITER_INTERFACE_H_
#define _BSDIFF_PATCH_WRITER_INTERFACE_H_

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

  // Initialize the patch writer old and new files with the given buffers.
  // These buffers are not copied into this class and must be valid until
  // Close() is called. They will be used to encode the extra and diff streams
  // when calling AddControlEntry(). Returns whether the initialization
  // succeeded.
  virtual bool InitializeBuffers(const uint8_t* old_buf,
                                 uint64_t old_size,
                                 const uint8_t* new_buf,
                                 uint64_t new_size) = 0;

  // Add a new control triplet entry to the patch. The |entry.diff_size| bytes
  // for the diff stream and the |entry.extra_size| bytes for the extra stream
  // will be computed and added to the corresponding streams in the patch.
  // Returns whether the operation succeeded. The operation can fail if either
  // the old or new files are referenced out of bounds.
  virtual bool AddControlEntry(const ControlEntry& entry) = 0;

  // Finalize the patch writing process and close the file.
  virtual bool Close() = 0;

 protected:
  PatchWriterInterface() = default;
};

}  // namespace bsdiff

#endif  // _BSDIFF_PATCH_WRITER_INTERFACE_H_
