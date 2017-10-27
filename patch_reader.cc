// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/patch_reader.h"

#include <string.h>

#include <limits>

#include "bsdiff/bspatch.h"
#include "bsdiff/bz2_decompressor.h"
#include "bsdiff/logging.h"
#include "bsdiff/utils.h"

using std::endl;

namespace bsdiff {

const uint8_t kMagicHeader[] = "BSDIFF40";

bool BsdiffPatchReader::Init(const uint8_t* patch_data, size_t patch_size) {
  // File format:
  //   0       8    "BSDIFF40"
  //   8       8    X
  //   16      8    Y
  //   24      8    new_file_size
  //   32      X    compressed control block
  //   32+X    Y    compressed diff block
  //   32+X+Y  ???  compressed extra block
  // with control block a set of triples (x,y,z) meaning "add x bytes
  // from oldfile to x bytes from the diff block; copy y bytes from the
  // extra block; seek forwards in oldfile by z bytes".

  // Check for appropriate magic.
  if (memcmp(patch_data, kMagicHeader, 8) != 0) {
    LOG(ERROR) << "Not a bsdiff patch." << endl;
    return false;
  }

  // Read lengths from header.
  int64_t ctrl_len = ParseInt64(patch_data + 8);
  int64_t diff_len = ParseInt64(patch_data + 16);
  int64_t signed_newsize = ParseInt64(patch_data + 24);
  if ((ctrl_len < 0) || (diff_len < 0) || (signed_newsize < 0) ||
      (32 + ctrl_len + diff_len > static_cast<int64_t>(patch_size))) {
    LOG(ERROR) << "Corrupt patch.  ctrl_len: " << ctrl_len
               << ", data_len: " << diff_len
               << ", new_file_size: " << signed_newsize
               << ", patch_size: " << patch_size;
    return false;
  }
  new_file_size_ = signed_newsize;

  // TODO(xunchang) set the correct decompressor based on the info in the
  // header.
  ctrl_stream_.reset(new BZ2Decompressor());
  diff_stream_.reset(new BZ2Decompressor());
  extra_stream_.reset(new BZ2Decompressor());

  int64_t offset = 32;
  if (!ctrl_stream_->SetInputData(const_cast<uint8_t*>(patch_data) + offset,
                                  ctrl_len)) {
    LOG(ERROR) << "Failed to init ctrl stream, ctrl_len: " << ctrl_len << endl;
    return false;
  }

  offset += ctrl_len;
  if (!diff_stream_->SetInputData(const_cast<uint8_t*>(patch_data) + offset,
                                  diff_len)) {
    LOG(ERROR) << "Failed to init ctrl stream, diff_len: " << diff_len << endl;
    return false;
  }

  offset += diff_len;
  if (!extra_stream_->SetInputData(const_cast<uint8_t*>(patch_data) + offset,
                                   patch_size - offset)) {
    LOG(ERROR) << "Failed to init extra stream, extra_offset: " << offset
               << ", patch_size: " << patch_size << endl;
    return false;
  }
  return true;
}

bool BsdiffPatchReader::ParseControlEntry(ControlEntry* control_entry) {
  if (!control_entry)
    return false;

  uint8_t buf[8];
  if (!ctrl_stream_->Read(buf, 8))
    return false;
  int64_t diff_size = ParseInt64(buf);

  if (!ctrl_stream_->Read(buf, 8))
    return false;
  int64_t extra_size = ParseInt64(buf);

  // Sanity check.
  if (diff_size < 0 || extra_size < 0) {
    LOG(ERROR) << "Corrupt patch; diff_size: " << diff_size
               << ", extra_size: " << extra_size << endl;
    return false;
  }

  control_entry->diff_size = diff_size;
  control_entry->extra_size = extra_size;

  if (!ctrl_stream_->Read(buf, 8))
    return false;
  control_entry->offset_increment = ParseInt64(buf);

  return true;
}

bool BsdiffPatchReader::ReadDiffStream(uint8_t* buf, size_t size) {
  return diff_stream_->Read(buf, size);
}

bool BsdiffPatchReader::ReadExtraStream(uint8_t* buf, size_t size) {
  return extra_stream_->Read(buf, size);
}

bool BsdiffPatchReader::Finish() {
  if (!ctrl_stream_->Close()) {
    LOG(ERROR) << "Failed to close the control stream";
    return false;
  }

  if (!diff_stream_->Close()) {
    LOG(ERROR) << "Failed to close the diff stream";
    return false;
  }

  if (!extra_stream_->Close()) {
    LOG(ERROR) << "Failed to close the extra stream";
    return false;
  }
  return true;
}

}  // namespace bsdiff
