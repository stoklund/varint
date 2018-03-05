Variable-Length Integers
========================

This repository contains code for exploring the performance of different
variable-length integer encoding schemes, specifically to determine the best
encoding to use in the WebAssembly binary encoding. See the related
[WebAssembly design issue](https://github.com/WebAssembly/design/issues/601).

There are three primary interesting performance factors for a variable-length
integer encoding:

1. Compression ratio for the relevant distribution of integers.
2. Decoding speed.
3. Encoding speed.

We don't measure encoding speed here since it is less relevant for WebAssembly
as long as it is reasonable.

Other desirable properties of a variable-length encoding:

- Can the length of the whole encoded integer be determined from the first byte?
  This makes it faster to skip over numbers that are not needed, and it can
  improve instruction-level parallelism when decoding multiple consequtive
  numbers.
- Patchability. Is it possible to reserve space for an unknown number, and write
  it back later? This requires a non-canonical encoding where a small number can
  be encoded with a larger length than strictly necessary.

Note that the C++ code in this repository is **not production quality**, and it
can be tricked into executing _undefined behavior_ by maliciously formed input.


LEB128
------
The [Little Endian Base-128](https://en.wikipedia.org/wiki/LEB128) encoding is
used in [DWARF](http://dwarfstd.org) debug information files and many other
places. It is a simple byte-oriented encoding where each byte contains 7 bits
of the integer being encoded, and the MSB is used as a _tag bit_ which
indicates if there are more bytes coming.

    0xxxxxxx                     7 bits in 1 byte
    1xxxxxxx 0xxxxxxx           14 bits in 2 bytes
    1xxxxxxx 1xxxxxxx 0xxxxxxx  21 bits in 3 bytes
    ...


PrefixVarint
------------
Brought up in WebAssembly/design#601, and probably invented independently many
times, this encoding is very similar to LEB128, but it moves all the tag bits
to the LSBs of the first byte:

    xxxxxxx1  7 bits in 1 byte
    xxxxxx10 14 bits in 2 bytes
    xxxxx100 21 bits in 3 bytes
    xxxx1000 28 bits in 4 bytes
    xxx10000 35 bits in 5 bytes
    xx100000 42 bits in 6 bytes
    x1000000 49 bits in 7 bytes
    10000000 56 bits in 8 bytes
    00000000 64 bits in 9 bytes

This has advantages on modern CPUs with fast unaligned loads and _count
trailing zeros_ instructions. The compression ratio is the same as for LEB128,
except for those 64-bit numbers that require 10 bytes to encode in LEB128.

Like [UTF-8](https://en.wikipedia.org/wiki/UTF-8), the length of a
PrefixVarint-encoded number can be determined from the first byte. (UTF-8 is
not considered here since it only encodes 6 bits per byte due to design
constraints that are not relevant to WebAssembly.)


leSQLite
--------
The [SQLite variable-length integer
encoding](https://sqlite.org/src4/doc/trunk/www/varint.wiki) is biased towards
integer distributions with more small numbers. It can encode the integers 0-240
in one byte.

The encoding implemented here is modified for better performance with
WebAssembly (little-endian SQLite). The first byte, `B0` determines the
encoding:

    0-184   1 byte    value = B0
    185-248 2 bytes   value = 185 + 256 * (B0 - 185) + B1
    249-255 3-9 bytes value = (B0 - 249 + 2) little-endian bytes following B0.

This encoding packs more than 7 bits into 1 byte and a bit more than 14 bits
into 2 bytes. This has a cost in encoding size since the 3-byte encoding only
holds 16 bits. The 3+ byte encoded numbers are very fast to decode with an
unaligned load instruction.


leSQLite2
---------
A second variation of the SQLite-inspired encoding has a smoother bump between
the 2-byte and 3-byte encodings. It divides the values of the first byte into 4
ranges:

1. The value of a 1-byte encoding.
2. The high 6 bits of a 2-byte encoding.
3. The high 3 bits of a 3-byte encoding.
4. The number of bytes in a 4-9 byte encoding.

The ranges are assigned like this:

| B0      | Values     | Formula                              |
| ------- | ---------- | ------------------------------------ |
| 0-177   | 177        | `B0`                                 |
| 178-241 | 2^14       | `178 + ((B0-178) << 8) + B[1]`       |
| 242-249 | 2^19       | `16562 + ((B0-242) << 16) + B[1..2]` |
| 250-255 | 2^24..2^64 | `B0 - 250 + 3` little-endian bytes.  |

This variant is a bit slower to decode than the first one because there are more
cases.
