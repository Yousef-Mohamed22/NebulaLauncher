#include <gtest/gtest.h>


#include <Platform/Arena.hpp>

TEST(Arena, Create)
{
    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    EXPECT_GE(arena.capacity(), 1024);
}

TEST(Arena, PushAndGet)
{
    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());

    int* value;
    EXPECT_TRUE(arena.allocate(&value).success());
    *value = 42;

    void* ptr;
    EXPECT_TRUE(arena.get(0, &ptr).success());
    EXPECT_EQ(*static_cast<int*>(ptr), 42);
}
