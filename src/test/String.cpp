#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

#include <gtest/gtest.h>

TEST(StringView, ImplicitCast) {

    nbl::StringView sv = "Hello";
    ASSERT_EQ(sv.length, 5);
    ASSERT_EQ(memcmp("Hello", sv.data, 5), 0);

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

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringView result;
    EXPECT_EQ(nbl::StringBuilder(&frame)
        .append("Hello World")
        .replace(' ', "_")
        .buildNullTerminated(&result), nbl::Result::SUCCESS);

    ASSERT_EQ(result, nbl::StringView("Hello_World"));

}
TEST(StringBuilder, ReplaceLarger) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringView result;
    EXPECT_EQ(nbl::StringBuilder(&frame)
        .append("Hello World")
        .replace('l', "___")
        .buildNullTerminated(&result), nbl::Result::SUCCESS);

    ASSERT_EQ(result, nbl::StringView("He______o Wor___d"));

}
