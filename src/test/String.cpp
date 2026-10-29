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


#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <Platform/Arena.hpp>
#include <Utility/StringBuilder.hpp>

#include <gtest/gtest.h>

TEST(StringView, ImplicitCast) {

    nbl::StringView sv = "Hello";
    ASSERT_EQ(sv.length(), 5);
    ASSERT_EQ(memcmp("Hello", sv.data(), 5), 0);

}

TEST(StringView, EndsWith) {

    nbl::StringView sv = "Hello";
    ASSERT_TRUE(sv.endsWith("Hello"));
    ASSERT_TRUE(sv.endsWith("ello"));
    ASSERT_TRUE(sv.endsWith("llo"));
    ASSERT_TRUE(sv.endsWith("lo"));
    ASSERT_TRUE(sv.endsWith("o"));
    ASSERT_TRUE(sv.endsWith(""));

    ASSERT_FALSE(sv.endsWith("H"));
    ASSERT_FALSE(sv.endsWith("World"));

}

TEST(StringBuilder, Replace) {

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());

    nbl::StringView result;
    EXPECT_EQ(nbl::StringBuilder(&frame)
        .append("Hello World")
        .replace(' ', "_")
        .buildNullTerminated(&result), nbl::Result::SUCCESS);

    ASSERT_EQ(result, nbl::StringView("Hello_World"));

}
TEST(StringBuilder, ReplaceLarger) {

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());

    nbl::StringView result;
    EXPECT_EQ(nbl::StringBuilder(&frame)
        .append("Hello World")
        .replace('l', "___")
        .buildNullTerminated(&result), nbl::Result::SUCCESS);

    ASSERT_EQ(result, nbl::StringView("He______o Wor___d"));

}
