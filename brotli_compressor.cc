// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/brotli_compressor.h"

#include "bsdiff/logging.h"

using std::endl;

namespace {
// TODO(xunchang) set brotli compression parameters based on input options.
const size_t kBufferSize = 1024 * 1024;
const uint32_t kBrotliQuality = 11;
const uint32_t kBrotliLgwin = 24;

}  // namespace

namespace bsdiff {

BrotliCompressor::BrotliCompressor() : comp_buffer_(kBufferSize) {
  brotli_encoder_state_ =
      BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
  if (!brotli_encoder_state_) {
    LOG(ERROR) << "Failed to initialize brotli decoder state" << endl;
  } else {
    BrotliEncoderSetParameter(brotli_encoder_state_, BROTLI_PARAM_QUALITY,
                              kBrotliQuality);
    BrotliEncoderSetParameter(brotli_encoder_state_, BROTLI_PARAM_LGWIN,
                              kBrotliLgwin);
  }
}

BrotliCompressor::~BrotliCompressor() {
  if (brotli_encoder_state_) {
    BrotliEncoderDestroyInstance(brotli_encoder_state_);
  }
}

bool BrotliCompressor::Write(const uint8_t* buf, size_t size) {
  if (!brotli_encoder_state_)
    return false;

  const uint8_t* next_in = buf;
  size_t avail_in = size;
  while (avail_in > 0) {
    size_t avail_out = kBufferSize;
    uint8_t* next_out = comp_buffer_.buffer_data();
    if (!BrotliEncoderCompressStream(
            brotli_encoder_state_, BROTLI_OPERATION_PROCESS, &avail_in,
            &next_in, &avail_out, &next_out, nullptr)) {
      LOG(ERROR) << "BrotliCompressor failed to compress " << avail_in
                 << " bytes of data." << endl;
      return false;
    }

    uint64_t output_bytes = comp_buffer_.buffer_size() - avail_out;
    if (output_bytes > 0) {
      comp_buffer_.AddDataToChunks(output_bytes);
    }
  }
  return true;
}

bool BrotliCompressor::Finish() {
  if (!brotli_encoder_state_)
    return false;

  const uint8_t* next_in = nullptr;
  size_t avail_in = 0;
  while (!BrotliEncoderIsFinished(brotli_encoder_state_)) {
    size_t avail_out = kBufferSize;
    uint8_t* next_out = comp_buffer_.buffer_data();
    if (!BrotliEncoderCompressStream(
            brotli_encoder_state_, BROTLI_OPERATION_FINISH, &avail_in, &next_in,
            &avail_out, &next_out, nullptr)) {
      LOG(ERROR) << "BrotliCompressor failed to finish compression" << endl;
      return false;
    }

    uint64_t output_bytes = comp_buffer_.buffer_size() - avail_out;
    if (output_bytes > 0) {
      comp_buffer_.AddDataToChunks(output_bytes);
    }
  }
  return true;
}

const std::vector<uint8_t>& BrotliCompressor::GetCompressedData() {
  return comp_buffer_.GetCompressedData();
}

}  // namespace bsdiff
