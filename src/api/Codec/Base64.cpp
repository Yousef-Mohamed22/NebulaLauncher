/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * \file Codec/Base64.cpp
 * \author jwall64
 * \brief Implementation of Codec/Base64.hpp
 */

#include <Codec/Base64.hpp>

#include <cstring>

namespace {

constexpr char B64_LOOKUP[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

constexpr char B64_REVERSE_LOOKUP[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  62, 0,  0,  0,  63, 52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0, 
    63, 0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,
};


constexpr char B64URL_LOOKUP[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

constexpr char B64URL_REVERSE_LOOKUP[256] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  62, 0,  0,  52, 53, 54, 55, 56, 57, 58, 59, 60,
    61, 0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0, 
    63, 0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
    43, 44, 45, 46, 47, 48, 49, 50, 51, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
    0,  0,  0,  0,  0,  0,  0,  0,  0,
};

}

nbl::Base64CharacterSet nbl::Base64CharacterSet::STANDARD = { .encodingTable = B64_LOOKUP, .decodingTable = B64_REVERSE_LOOKUP };
nbl::Base64CharacterSet nbl::Base64CharacterSet::URL_SAFE = { .encodingTable = B64URL_LOOKUP, .decodingTable = B64URL_REVERSE_LOOKUP };


nbl::Result nbl::encodeBase64(Span<u8> input, Span<char> output, const Base64CharacterSet& characterSet) {

    if(output.length < b64EncodedLength(input.length)) return nbl::Result::INVALID_ARGUMENT;

    usize dataLength = input.length;
    usize index = 0;

    usize position = 0;
    while(dataLength - position >= 3) {

        u32 encodedChunk = 
            (input.data[position] << 16) |
            (input.data[position + 1] << 8) |
            (input.data[position + 2]);

        position += 3;

        u8 c1 = encodedChunk & 0x3F;
        u8 c2 = (encodedChunk >> 6) & 0x3F;
        u8 c3 = (encodedChunk >> 12) & 0x3F;
        u8 c4 = (encodedChunk >> 18) & 0x3F;

        char chunk[4] = { 
            characterSet.encodingTable[c4], 
            characterSet.encodingTable[c3], 
            characterSet.encodingTable[c2], 
            characterSet.encodingTable[c1] 
        };

        memcpy(output.data + index, chunk, 4);
        index += 4;
    }

    usize remaining = dataLength - position;
    if(remaining == 2) {

        u32 encodedChunk = 
            ((input.data[position] << 8) |
            (input.data[position + 1])) << 2;

        position += 2;

        u8 c1 = encodedChunk & 0x3F;
        u8 c2 = (encodedChunk >> 6) & 0x3F;
        u8 c3 = (encodedChunk >> 12) & 0x3F;

        char chunk[3] = {
            characterSet.encodingTable[c3],
            characterSet.encodingTable[c2],
            characterSet.encodingTable[c1]
        };

        memcpy(output.data + index, chunk, 3);
        index += 3;
    }
    else if(remaining == 1) {
        u32 encodedChunk = 
            (input.data[position]) << 4;

        position += 1;

        u8 c1 = encodedChunk & 0x3F;
        u8 c2 = (encodedChunk >> 6) & 0x3F;

        char chunk[2] = {
            characterSet.encodingTable[c2],
            characterSet.encodingTable[c1]
        };
        memcpy(output.data + index, chunk, 2);
        index += 2;
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::decodeBase64(Span<char> input, Span<u8> output, const Base64CharacterSet& characterSet) {

    if(output.length < b64DecodedLength(input.length)) return nbl::Result::INVALID_ARGUMENT;

    usize position = 0;
    usize encodedLength = input.length;
    usize index = 0;

    while((encodedLength - position) >= 4) {

        u8 c1 = B64URL_REVERSE_LOOKUP[input.data[position]];
        u8 c2 = B64URL_REVERSE_LOOKUP[input.data[position + 1]];
        u8 c3 = B64URL_REVERSE_LOOKUP[input.data[position + 2]];
        u8 c4 = B64URL_REVERSE_LOOKUP[input.data[position + 3]];

        position += 4;

        u32 chunk = (c1 << 18) | (c2 << 12) | (c3 << 6) | c4;
        u8 data[3] = { 
            static_cast<u8>((chunk >> 16) & 0xFF), 
            static_cast<u8>((chunk >> 8) & 0xFF), 
            static_cast<u8>(chunk & 0xFF) 
        };
        memcpy(output.data + index, data, 3);
        index += 3;
    }

    usize remaining = encodedLength - position;
    if(remaining == 3) {
        u8 c1 = B64URL_REVERSE_LOOKUP[input.data[position]];
        u8 c2 = B64URL_REVERSE_LOOKUP[input.data[position + 1]];
        u8 c3 = B64URL_REVERSE_LOOKUP[input.data[position + 2]];

        position += 3;

        u32 chunk = ((c1 << 12) | (c2 << 6) | c3) >> 2;
        u8 data[2] = { 
            static_cast<u8>((chunk >> 8) & 0xFF), 
            static_cast<u8>(chunk & 0xFF) 
        };
        memcpy(output.data + index, data, 2);
        index += 2;

    } else if(remaining == 2) {
        u8 c1 = B64URL_REVERSE_LOOKUP[input.data[position]];
        u8 c2 = B64URL_REVERSE_LOOKUP[input.data[position + 1]];

        position += 2;

        u32 chunk = ((c1 << 6) | c2) >> 4;
        u8 data = static_cast<u8>(chunk & 0xFF);

        output.data[index++] = data;

    } else if(remaining == 1) {
        u8 c1 = B64URL_REVERSE_LOOKUP[input.data[position]];
        position += 1;

        output.data[index++] = c1;
    }

    return nbl::Result::SUCCESS;
}

usize nbl::b64EncodedLength(usize dataLength) {
    size_t chunks = dataLength / 3;
    size_t rem = dataLength % 3;
    return (4 * chunks) + ((rem > 0) * (rem + 1));
}

usize nbl::b64DecodedLength(usize encodedLength) {
    if(encodedLength <= 1) return encodedLength;
    size_t chunks = encodedLength / 4;
    size_t rem = encodedLength % 4;
    return (chunks * 3) + ((rem > 0) * (rem - 1));
}
