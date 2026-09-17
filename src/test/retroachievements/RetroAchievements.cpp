#include <gtest/gtest.h>
#include <Core/Array.hpp>

#include "../../plugin_retroachievements/Utility.hpp"
#include "Core/String.hpp"

void _check(nbl::StringView unparsed, nbl::StringView expectedTitle, nbl::StringView expectedSubset, nbl::Span<nbl::StringView> expectedTags) {

    usize index = 0;
    auto _consume = [&](nbl::StringView tag) {
        EXPECT_EQ(tag, expectedTags[index++]);
    };

    nbl::StringView outTitle{};
    nbl::StringView outSubset{};
    NBL_ASSERT(nbl::ra::parseTitle(unparsed, _consume, &outTitle, &outSubset).success());

    EXPECT_EQ(outTitle, expectedTitle);
    EXPECT_EQ(outSubset, expectedSubset);

}

TEST(RetroAchivements, ParseRealTitles) {

    _check("Super Mario Galaxy", "Super Mario Galaxy", {}, {});
    _check("Super Mario Galaxy 2", "Super Mario Galaxy 2", {}, {});
    _check("~Hack~ Super Mario Galaxy 63", "Super Mario Galaxy 63", {}, nbl::Array { nbl::StringView("Hack") });
    _check("~Homebrew~ ~Demo~ Super Mario Galaxy DS", "Super Mario Galaxy DS", {}, nbl::Array { nbl::StringView("Homebrew"), nbl::StringView("Demo") });
    _check("Super Mario Galaxy [Subset - Coin Collector]", "Super Mario Galaxy", "Coin Collector", {});
    _check("Super Mario Galaxy 2 [Subset - Time Attack]", "Super Mario Galaxy 2", "Time Attack", {});
}

TEST(RetroAchievements, ParseTheoreticalTitles) {
    _check("~Tag~ And Title [Subset - And Subset]", "And Title", "And Subset", nbl::Array { nbl::StringView("Tag") });
    _check("Title[With Brackets]", "Title[With Brackets]", {}, {});
    _check("Title[With Brackets] [And more brackets]", "Title[With Brackets] [And more brackets]", {}, {});
    _check("Title[With Brackets] [Subset - And Subset]", "Title[With Brackets]", "And Subset", {});
    _check("Title[With Brackets] [And more brackets] [Subset - And Subset]", "Title[With Brackets] [And more brackets]", "And Subset", {});
    _check("~One Tag~ ~Two Tags~ ~Three Tags~ Title[With Brackets] [And more brackets] [Subset - And Subset]", "Title[With Brackets] [And more brackets]", "And Subset", nbl::Array { nbl::StringView("One Tag"), nbl::StringView("Two Tags"), nbl::StringView("Three Tags") } );
}

TEST(RetroAchievements, ParseDate) {

    nbl::StringView date = "2026-10-01 03:48:22";
    
    u64 outTimeStamp;
    ASSERT_EQ(nbl::ra::parseDate(date, &outTimeStamp), nbl::Result::SUCCESS);
    ASSERT_EQ(outTimeStamp, 1790826502000);

}
