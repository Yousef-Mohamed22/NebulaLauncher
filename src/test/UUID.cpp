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


#include <Utility/UUID.hpp>
#include <Core/Array.hpp>
#include <Core/Stream.hpp>
#include <Core/String.hpp>

#include <gtest/gtest.h>

TEST(UUID, Version) {

    nbl::UUID v4 = nbl::UUID::generateV4();
    nbl::UUID v7 = nbl::UUID::generateV7();

    EXPECT_EQ(4, v4.version());
    EXPECT_EQ(7, v7.version());
}

TEST(UUID, Write) {

    nbl::UUID uuid {
        .msb = 0xbf45f994826847a5,
        .lsb = 0x983d01661490f9f3
    };

    nbl::Array<char, 36> buffer{};


    nbl::BufferStream stream(buffer.span());
    ASSERT_EQ(nbl::toString(uuid, stream), nbl::Result::SUCCESS);
    ASSERT_EQ(nbl::StringView(buffer.data, buffer.size()), nbl::StringView("bf45f994-8268-47a5-983d-01661490f9f3"));
}

TEST(UUID, Generate) {

    nbl::Random rand{};
    rand.seed(42);

    nbl::UUID uuid = nbl::UUID::generateV4(rand);

    nbl::UUID exp {
        .msb = 0x84B5B1D2F8014CAF,
        .lsb = 0xBDAACCD92E03FC20,
    };

    ASSERT_EQ(exp, uuid);

}
