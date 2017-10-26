// Copyright 2015 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <err.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <limits>

#include "bsdiff/bsdiff.h"
#include "bsdiff/patch_writer_factory.h"

namespace {

// mmap() the passed |filename| to read-only memory and store in |filesize| the
// size of the file. To release the memory, call munmap with the returned
// pointer and filesize. In case of error returns nullptr.
void* MapFile(const char* filename, size_t* filesize) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open()");
    return nullptr;
  }

  struct stat st;
  fstat(fd, &st);
  if (static_cast<uint64_t>(st.st_size) > std::numeric_limits<size_t>::max()) {
    fprintf(stderr, "File too big\n");
    close(fd);
    return nullptr;
  }
  *filesize = st.st_size;

  void* ret = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (ret == MAP_FAILED) {
    perror("mmap()");
    close(fd);
    return nullptr;
  }
  close(fd);
  return ret;
}

// Generate bsdiff patch from the |old_filename| file to the |new_filename|
// file storing the resulting patch in a new |patch_filename| file.
int GenerateBsdiffFromFiles(const char* old_filename,
                            const char* new_filename,
                            const char* patch_filename) {
  size_t oldsize, newsize;
  int ret = 0;

  uint8_t* old_buf = static_cast<uint8_t*>(MapFile(old_filename, &oldsize));
  uint8_t* new_buf = static_cast<uint8_t*>(MapFile(new_filename, &newsize));

  if (old_buf && new_buf) {
    auto patch_writer = bsdiff::CreateBsdiffPatchWriter(patch_filename);

    ret = bsdiff::bsdiff(old_buf, oldsize, new_buf, newsize, patch_writer.get(),
                         nullptr);
  } else {
    ret = 1;
  }

  if (old_buf)
    munmap(old_buf, oldsize);
  if (new_buf)
    munmap(new_buf, newsize);

  return ret;
}

}  // namespace

int main(int argc, char* argv[]) {
  if (argc != 4)
    errx(1, "usage: %s oldfile newfile patchfile\n", argv[0]);

  return GenerateBsdiffFromFiles(argv[1], argv[2], argv[3]);
}
