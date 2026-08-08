#include <Utility/Placeholder.hpp>
#include <Core/Array.hpp>

#include <gtest/gtest.h>


TEST(Placeholder, ParseNone) {

    nbl::StringView format = "Hello, World";
    nbl::StringView done{};

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, {}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(format, done);
}

TEST(Placeholder, ParseAtEnd) {

    nbl::StringView format = "Hello, ${target}";
    nbl::StringView done{};

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{nbl::Placeholder{.key="target", .value="World"}}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseAtBeginning) {

    nbl::StringView format = "${greeting}, World";
    nbl::StringView done{};

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{nbl::Placeholder{.key="greeting", .value="Hello"}}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseInMiddle) {

    nbl::StringView format = "Hello${separator}World";
    nbl::StringView done{};

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{nbl::Placeholder{.key="separator", .value=", "}}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseOnlyPlaceholder) {

    nbl::StringView format = "${greeting}";
    nbl::StringView done{};

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{
                nbl::Placeholder{.key="greeting", .value="Hello, World"},
    }), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseMultiple) {

    nbl::StringView format = "${greeting}, ${target}";
    nbl::StringView done{};

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{
                nbl::Placeholder{.key="greeting", .value="Hello"},
                nbl::Placeholder{.key="target", .value="World"}
    }), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}
