// Copyright 2016 Jakob Stoklund Olesen
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This implements the standard LEB128 codec.

#include "compiler.h"
#include "varint.h"

using namespace std;

vector<uint8_t> prefix_encode(const vector<uint64_t> &in) {
  vector<uint8_t> out;
  for (auto x : in) {
    unsigned bits = 64 - count_leading_zeros_64(x | 1);
    unsigned bytes = 1 + (bits - 1) / 7;
    if (bits > 56) {
      out.push_back(0);
      bytes = 8;
    } else {
      x = (2 * x + 1) << (bytes - 1);
    }
    for (unsigned n = 0; n < bytes; n++) {
      out.push_back(x & 0xff);
      x >>= 8;
    }
  }
  return out;
}

void prefix_decode(const uint8_t *in, uint64_t *out, size_t count) {
  while (count-- > 0) {
    uint64_t v = unaligned_load_u64(in);
    size_t length = count_trailing_zeros_32(v);
    if (LIKELY(length < 8)) {
      uint64_t m = -(uint64_t)256 << (length * 8);
      length++;
      *out++ = (v & ~m) >> length;
      in += length;
    } else {
      *out++ = unaligned_load_u64(in + 1);
      in += 9;
    }
  }
}

const codec_descriptor prefix_codec = {
    .name = "PrefixVarint", .encoder = prefix_encode, .decoder = prefix_decode,
};
