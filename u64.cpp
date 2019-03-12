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

// This implements the u64 codec: just pass-thru uint64_t for comparison.

#include "compiler.h"
#include "varint.h"

using namespace std;

vector<uint8_t> u64_encode(const vector<uint64_t> &in) {
  vector<uint8_t> out;
  for (auto x : in) {
    out.insert(out.end(), reinterpret_cast<uint8_t *>(&x), reinterpret_cast<uint8_t *>(&x) + sizeof(uint64_t));
  }
  return out;
}

void u64_decode(const uint8_t *in, uint64_t *out, size_t count) {
  while (count-- > 0) {
    std::memcpy(out, in, sizeof(uint64_t));
    out++;
    in += sizeof(uint64_t);
  }
}

const codec_descriptor u64_codec = {
    .name = "U64", .encoder = u64_encode, .decoder = u64_decode,
};
