#include <Core/Random.hpp>
#include <gtest/gtest.h>

TEST(Random, Simple) {

    nbl::Random random{};
    random.seed(6969);

    ASSERT_EQ(random.next() % 10, 0);
    ASSERT_EQ(random.next() % 10, 7);
    ASSERT_EQ(random.next() % 10, 0);
    ASSERT_EQ(random.next() % 10, 5);
    ASSERT_EQ(random.next() % 10, 8);
}
