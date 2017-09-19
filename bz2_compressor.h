// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _BSDIFF_BZ2_COMPRESSOR_H_
#define _BSDIFF_BZ2_COMPRESSOR_H_

#include <bzlib.h>
#include <stdint.h>

#include <vector>

namespace bsdiff {

// An in-memory class to wrap the low-level bzip2 compress functions. This class
// allows to stream uncompressed data to it and then retrieve all the compressed
// data at the end of the compression step. For that, all the compressed data
// is stored in memory.

class BZ2Compressor {
 public:
  BZ2Compressor();
  ~BZ2Compressor();

  // Compress and write the data at |buf| of size |size|.
  bool Write(const uint8_t* buf, size_t size);

  // Finish the compression step.
  bool Finish();

  // Return the compressed data. This method must be only called after Finish().
  const std::vector<uint8_t>& GetCompressedData();

 private:
  // The low-level bzip2 stream.
  bz_stream bz_strm_;

  // Whether the bz_strm_ is initialized.
  bool bz_strm_initialized_{false};

  // A list of chunks of compressed data. The final compressed representation is
  // the concatenation of all the compressed data.
  std::vector<std::vector<uint8_t>> comp_chunks_;

  // A concatenated version of the |comp_chunks_|, used to store the compressed
  // memory after Finish() is called.
  std::vector<uint8_t> comp_data_;

  // A temporary compression buffer for multiple calls to Write().
  std::vector<uint8_t> comp_buffer_;
};

}  // namespace bsdiff

#endif  // _BSDIFF_BZ2_COMPRESSOR_H_
