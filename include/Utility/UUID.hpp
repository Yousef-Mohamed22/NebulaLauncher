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
