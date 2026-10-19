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
 * \file Utility/UUID.hpp
 * \author jwall64
 * \brief Utility for creating UUIDs
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Core/Random.hpp>
#include <Platform/Time.hpp>

namespace nbl {

struct UUID {
    u64 msb;
    u64 lsb;

    constexpr u8 version() const {
        return (msb >> 12) & 0xF;
    }

    constexpr bool operator==(const UUID& other) const {
        return msb == other.msb && lsb == other.lsb;
    }

    inline static UUID generateV4() {
        Random rand{};
        rand.seed(nbl::getCurrentTime());
        return generateV4(rand);
    }
    static UUID generateV4(Random random);

    inline static UUID generateV7() {
        Random rand{};
        rand.seed(nbl::getCurrentTime());
        return generateV7(rand);
    }
    static UUID generateV7(Random random);
};

Result toString(UUID uuid, Writer writer);


}
