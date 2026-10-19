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


#include <Utility/Placeholder.hpp>
#include <Core/Array.hpp>
#include <Platform/Arena.hpp>

#include <gtest/gtest.h>


TEST(Placeholder, ParseNone) {

    nbl::StringView format = "Hello, World";
    nbl::StringView done{};

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, {}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(format, done);
}

TEST(Placeholder, ParseAtEnd) {

    nbl::StringView format = "Hello, ${target}";
    nbl::StringView done{};

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{nbl::Placeholder{.key="target", .value="World"}}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseAtBeginning) {

    nbl::StringView format = "${greeting}, World";
    nbl::StringView done{};

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{nbl::Placeholder{.key="greeting", .value="Hello"}}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseInMiddle) {

    nbl::StringView format = "Hello${separator}World";
    nbl::StringView done{};

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{nbl::Placeholder{.key="separator", .value=", "}}), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}

TEST(Placeholder, ParseOnlyPlaceholder) {

    nbl::StringView format = "${greeting}";
    nbl::StringView done{};

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
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

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    ASSERT_EQ(nbl::formatPlaceholders(format, &builder, nbl::Array{
                nbl::Placeholder{.key="greeting", .value="Hello"},
                nbl::Placeholder{.key="target", .value="World"}
    }), nbl::Result::SUCCESS);
    ASSERT_EQ(builder.build(&done), nbl::Result::SUCCESS);

    ASSERT_EQ(done, "Hello, World");
}
