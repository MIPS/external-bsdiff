// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_PATCH_WRITER_H_
#define _BSDIFF_PATCH_WRITER_H_

#include <string>
#include <vector>

#include "bsdiff/bz2_compressor.h"
#include "bsdiff/patch_writer_interface.h"

namespace bsdiff {

// A PatchWriterInterface class using the upstream's BSDIFF40 format: three
// BZ2-compressors and a 32-byte header.
class BsdiffPatchWriter : public PatchWriterInterface {
 public:
  // Create the patch writer using the file |patch_filename| where the patch
  // data will be written to.
  explicit BsdiffPatchWriter(const std::string& patch_filename)
      : patch_filename_(patch_filename) {}

  // PatchWriterInterface overrides.
  bool Init() override;
  bool WriteDiffStream(const uint8_t* data, size_t size) override;
  bool WriteExtraStream(const uint8_t* data, size_t size) override;
  bool AddControlEntry(const ControlEntry& entry) override;
  bool Close() override;

 private:
  // Write the BSDIFF patch header to the |fp_| given the size of the compressed
  // control block and the compressed diff block.
  bool WriteHeader(uint64_t ctrl_size, uint64_t diff_size);

  // Bytes of the new files already written. Needed to store the new length in
  // the header of the file.
  uint64_t written_output_{0};

  // The current file we are writing to.
  FILE* fp_{nullptr};
  std::string patch_filename_;

  // The three internal compressed streams.
  BZ2Compressor ctrl_stream_;
  BZ2Compressor diff_stream_;
  BZ2Compressor extra_stream_;
};

}  // namespace bsdiff

#endif  // _BSDIFF_PATCH_WRITER_H_
