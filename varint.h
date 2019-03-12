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

#include <cstdint>
#include <vector>

typedef std::vector<uint8_t> (*encoder_func)(const std::vector<uint64_t>& in);
typedef void (*decoder_func)(const uint8_t* in, uint64_t* out, size_t count);

struct codec_descriptor {
    const char* name;
    encoder_func encoder;
    decoder_func decoder;
};

extern const codec_descriptor leb128_codec;
extern const codec_descriptor prefix_codec;
extern const codec_descriptor lesqlite_codec;
extern const codec_descriptor lesqlite2_codec;
extern const codec_descriptor u64_codec;
