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
 * \file Codec/Base64.hpp
 * \author jwall64
 * \brief Interface for encoding and decoding Base64
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Span.hpp>

namespace nbl {

struct Base64CharacterSet {

    const char* encodingTable;
    const char* decodingTable;

    static Base64CharacterSet STANDARD;
    static Base64CharacterSet URL_SAFE;
};

usize b64EncodedLength(usize length);
usize b64DecodedLength(usize length);

Result encodeBase64(Span<u8> input, Span<char> output, const Base64CharacterSet& characterSet = Base64CharacterSet::STANDARD);
Result decodeBase64(Span<char> input, Span<u8> output, const Base64CharacterSet& characterSet = Base64CharacterSet::STANDARD);


}
