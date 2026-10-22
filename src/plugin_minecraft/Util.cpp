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

#include "Util.hpp"

nbl::Result nbl::mc::parseHex(StringView hex, Span<u8> out) {
    if(hex.length % 2 != 0) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    if(hex.length / 2 > out.length) {
        return nbl::Result::INVALID_ARGUMENT;
    }

    for(usize i = 0 ; i < hex.length ; i++) {
        char hexChar = hex[i];
        if(!(hexChar >= '0' && hexChar <= '9')
        && !(hexChar >= 'a' && hexChar <= 'f')) {
            return nbl::Result::FORMAT_ERROR;
        }

        u8 hexValue = (hexChar >= '0' && hexChar <= '9') 
            ? hexChar - '0'
            : hexChar - 'a' + 10;

        usize byteIndex = i / 2;
        if(i % 2 == 0) {
            out[byteIndex] |= (hexValue << 4);
        } else {
            out[byteIndex] |= (hexValue & 0xF);
        }
    }

    return nbl::Result::SUCCESS;
}
