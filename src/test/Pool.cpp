#include <Platform/Pool.hpp>

#include <gtest/gtest.h>

TEST(Pool, PushAndGet) {

    nbl::Arena arena;
    ASSERT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::Pool pool;
    ASSERT_TRUE(nbl::Pool::create(&arena, sizeof(i32), &pool).success());

    i32* value1;
    ASSERT_EQ(pool.push((void**)&value1), nbl::Result::SUCCESS);
    *value1 = 42;

    i32* value2;
    ASSERT_EQ(pool.push((void**)&value2), nbl::Result::SUCCESS);
    *value2 = 84;

    void* outPtr;
    ASSERT_TRUE(pool.get(0, &outPtr).success());
    ASSERT_EQ(*static_cast<i32*>(outPtr), 42);

    ASSERT_TRUE(pool.get(1, &outPtr).success());
    ASSERT_EQ(*static_cast<i32*>(outPtr), 84);
}

TEST(Pool, OutOfBounds) {

    nbl::Arena arena;
    ASSERT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::Pool pool;
    ASSERT_TRUE(nbl::Pool::create(&arena, sizeof(i32), &pool).success());

    void* outPtr;
    ASSERT_EQ(pool.get(0, &outPtr), nbl::error::OUT_OF_BOUNDS);
}

TEST(PoolView, Iterate) {

    nbl::Arena arena;
    ASSERT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::PoolView<i32> pool;
    ASSERT_TRUE(nbl::PoolView<i32>::create(&arena, &pool).success());

    i32* value1;
    ASSERT_EQ(pool.push(&value1), nbl::Result::SUCCESS);
    *value1 = 42;

    i32* value2;
    ASSERT_EQ(pool.push(&value2), nbl::Result::SUCCESS);
    *value2 = 84;

    i32* value3;
    ASSERT_EQ(pool.push(&value3), nbl::Result::SUCCESS);
    *value3 = 168;

    usize index = 0;
    for(auto it = pool.begin(); it != pool.end(); ++it) {
        i32* value = *it;
        ASSERT_TRUE(value == value1 || value == value2 || value == value3);
        index++;
    }

    ASSERT_EQ(index, 3);

}

TEST(PoolView, PopAndIterateStart) {

    nbl::Arena arena;
    ASSERT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::PoolView<i32> pool;
    ASSERT_TRUE(nbl::PoolView<i32>::create(&arena, &pool).success());

    i32* value1;
    ASSERT_EQ(pool.push(&value1), nbl::Result::SUCCESS);
    *value1 = 42;

    i32* value2;
    ASSERT_EQ(pool.push(&value2), nbl::Result::SUCCESS);
    *value2 = 84;

    i32* value3;
    ASSERT_EQ(pool.push(&value3), nbl::Result::SUCCESS);
    *value3 = 168;

    ASSERT_EQ(pool.pop(value1), nbl::Result::SUCCESS);

    usize index = 0;
    for(auto it = pool.begin(); it != pool.end(); ++it) {
        i32* value = *it;
        ASSERT_TRUE(value == value2 || value == value3);
        ASSERT_FALSE(value == value1);
        index++;
    }

    ASSERT_EQ(index, 2);

}


TEST(PoolView, PopAndIterateMiddle) {

    nbl::Arena arena;
    ASSERT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::PoolView<i32> pool;
    ASSERT_TRUE(nbl::PoolView<i32>::create(&arena, &pool).success());

    i32* value1;
    ASSERT_EQ(pool.push(&value1), nbl::Result::SUCCESS);
    *value1 = 42;

    i32* value2;
    ASSERT_EQ(pool.push(&value2), nbl::Result::SUCCESS);
    *value2 = 84;

    i32* value3;
    ASSERT_EQ(pool.push(&value3), nbl::Result::SUCCESS);
    *value3 = 168;

    ASSERT_EQ(pool.pop(value2), nbl::Result::SUCCESS);

    usize index = 0;
    for(auto it = pool.begin(); it != pool.end(); ++it) {
        i32* value = *it;
        ASSERT_TRUE(value == value1 || value == value3);
        ASSERT_FALSE(value == value2);
        index++;
    }

    ASSERT_EQ(index, 2);

}


TEST(PoolView, PopAndIterateEnd) {

    nbl::Arena arena;
    ASSERT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::PoolView<i32> pool;
    ASSERT_TRUE(nbl::PoolView<i32>::create(&arena, &pool).success());

    i32* value1;
    ASSERT_EQ(pool.push(&value1), nbl::Result::SUCCESS);
    *value1 = 42;

    i32* value2;
    ASSERT_EQ(pool.push(&value2), nbl::Result::SUCCESS);
    *value2 = 84;

    i32* value3;
    ASSERT_EQ(pool.push(&value3), nbl::Result::SUCCESS);
    *value3 = 168;

    ASSERT_EQ(pool.pop(value3), nbl::Result::SUCCESS);

    usize index = 0;
    for(auto it = pool.begin(); it != pool.end(); ++it) {
        i32* value = *it;
        ASSERT_TRUE(value == value1 || value == value2);
        ASSERT_FALSE(value == value3);
        index++;
    }

    ASSERT_EQ(index, 2);

}
