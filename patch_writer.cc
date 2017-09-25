// Copyright 2017 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bsdiff/patch_writer.h"

#include <string.h>

#include "bsdiff/logging.h"

using std::endl;

namespace {

// The maximum positive number that we should encode. A number larger than this
// for unsigned fields will be interpreted as a negative value and thus a
// corrupt patch.
const uint64_t kMaxEncodedUint64Value = (1ULL << 63) - 1;

constexpr uint8_t kMagicHeader[] = "BSDIFF40";

void EncodeInt64(int64_t x, uint8_t* buf) {
  uint64_t y = x < 0 ? (1ULL << 63ULL) - x : x;
  for (int i = 0; i < 8; ++i) {
    buf[i] = y & 0xff;
    y /= 256;
  }
}

}  // namespace

namespace bsdiff {

bool BsdiffPatchWriter::Open(const std::string& patch_filename) {
  fp_ = fopen(patch_filename.c_str(), "w");
  if (!fp_) {
    LOG(ERROR) << "Opening " << patch_filename << endl;
    return false;
  }
  return true;
}

bool BsdiffPatchWriter::AddControlEntry(const ControlEntry& entry) {
  if (entry.diff_size > kMaxEncodedUint64Value) {
    LOG(ERROR) << "Encoding value out of range " << entry.diff_size << endl;
    return false;
  }

  if (entry.extra_size > kMaxEncodedUint64Value) {
    LOG(ERROR) << "Encoding value out of range " << entry.extra_size << endl;
    return false;
  }

  if (written_output_ + entry.diff_size + entry.extra_size > new_size_) {
    LOG(ERROR) << "Wrote more output than the declared new_size" << endl;
    return false;
  }

  if (entry.diff_size > 0 &&
      (old_pos_ < 0 || static_cast<uint64_t>(old_pos_) >= old_size_)) {
    LOG(ERROR) << "The pointer in the old stream (" << old_pos_
               << ") is out of bounds [0, " << old_size_ << ")" << endl;
    return false;
  }

  // Generate the 24 byte control entry.
  uint8_t buf[24];
  EncodeInt64(entry.diff_size, buf);
  EncodeInt64(entry.extra_size, buf + 8);
  EncodeInt64(entry.offset_increment, buf + 16);
  if (!ctrl_stream_.Write(buf, sizeof(buf)))
    return false;

  // Generate the diff stream.
  std::vector<uint8_t> diff(entry.diff_size);
  for (uint64_t i = 0; i < entry.diff_size; ++i) {
    diff[i] = new_buf_[written_output_ + i] - old_buf_[old_pos_ + i];
  }
  if (!diff_stream_.Write(diff.data(), diff.size()))
    return false;

  if (!extra_stream_.Write(new_buf_ + written_output_ + entry.diff_size,
                           entry.extra_size))
    return false;

  old_pos_ += entry.diff_size + entry.offset_increment;
  written_output_ += entry.diff_size + entry.extra_size;
  return true;
}

bool BsdiffPatchWriter::Close() {
  if (!fp_) {
    LOG(ERROR) << "File not open." << endl;
    return false;
  }
  if (written_output_ != new_size_) {
    LOG(ERROR) << "Close() called but not all the output was written" << endl;
    return false;
  }

  if (!ctrl_stream_.Finish() || !diff_stream_.Finish() ||
      !extra_stream_.Finish())
    return false;

  auto ctrl_data = ctrl_stream_.GetCompressedData();
  auto diff_data = diff_stream_.GetCompressedData();
  auto extra_data = extra_stream_.GetCompressedData();

  if (!WriteHeader(ctrl_data.size(), diff_data.size()))
    return false;

  if (fwrite(ctrl_data.data(), 1, ctrl_data.size(), fp_) != ctrl_data.size()) {
    LOG(ERROR) << "Writing ctrl_data." << endl;
    return false;
  }
  if (fwrite(diff_data.data(), 1, diff_data.size(), fp_) != diff_data.size()) {
    LOG(ERROR) << "Writing diff_data." << endl;
    return false;
  }
  if (fwrite(extra_data.data(), 1, extra_data.size(), fp_) !=
      extra_data.size()) {
    LOG(ERROR) << "Writing extra_data." << endl;
    return false;
  }
  if (fclose(fp_) != 0) {
    LOG(ERROR) << "Closing the patch file." << endl;
    return false;
  }
  fp_ = nullptr;
  return true;
}

bool BsdiffPatchWriter::WriteHeader(uint64_t ctrl_size, uint64_t diff_size) {
  /* Header format is
   * 0 8  "BSDIFF40"
   * 8 8 length of bzip2ed ctrl block
   * 16  8 length of bzip2ed diff block
   * 24  8 length of new file
   *
   * File format is
   * 0 32  Header
   * 32  ??  Bzip2ed ctrl block
   * ??  ??  Bzip2ed diff block
   * ??  ??  Bzip2ed extra block
   */
  uint8_t header[32];
  memcpy(header, kMagicHeader, 8);
  EncodeInt64(ctrl_size, header + 8);
  EncodeInt64(diff_size, header + 16);
  EncodeInt64(new_size_, header + 24);
  if (fwrite(header, sizeof(header), 1, fp_) != 1) {
    LOG(ERROR) << "writing to the patch file" << endl;
    return false;
  }
  return true;
}

}  // namespace bsdiff
