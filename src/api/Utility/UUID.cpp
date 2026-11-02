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
 * \file Utility/UUID.cpp
 * \author jwall64
 * \brief Implementation of Utility/UUID.hpp
 */

#include <Utility/UUID.hpp>

#include <Core/String.hpp>
#include <Core/Random.hpp>
#include <Core/Array.hpp>
#include <Platform/Time.hpp>

namespace {

constexpr u64 VERSION_MASK = 0xFFFFFFFFFFFF0FFFL;
constexpr u64 VARIANT_MASK = 0x3FFFFFFFFFFFFFFFL;

}

nbl::UUID nbl::UUID::generateV4(Random random) {
    u64 msb = random.next();
    u64 lsb = random.next();
    msb = 0x0000000000004000L | (msb & VERSION_MASK);
    lsb = 0x8000000000000000L | (lsb & VARIANT_MASK);
    return { .msb = msb, .lsb = lsb };
}

nbl::UUID nbl::UUID::generateV7(Random random) {
    u64 time = nbl::getCurrentTime();
    u64 msb = (time << 16) + (random.next() % 65535);
    u64 lsb = random.next();
    msb = 0x0000000000007000L | (msb & VERSION_MASK);
    lsb = 0x8000000000000000L | (lsb & VARIANT_MASK);
    return { .msb = msb, .lsb = lsb };
}


nbl::Result nbl::write(UUID uuid, Writer writer) {

    Array<u8, 36> hex{};

    nbl::writeHex(uuid.msb, BufferStream(hex.span()), 16, true);
    nbl::writeHex(uuid.lsb, BufferStream({ hex.data + 16 + 3, 16 }), 16, true);

    memmove(hex.data + 24, hex.data + 23, 12);
    memmove(hex.data + 14, hex.data + 12, 4);
    memmove(hex.data + 9,  hex.data + 8,  4);

    hex.data[8] = '-';
    hex.data[13] = '-';
    hex.data[18] = '-';
    hex.data[23] = '-';

    return writer.writeAll(hex.data, hex.size());
}
