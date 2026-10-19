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


#include <Platform/Pool.hpp>

#include <gtest/gtest.h>

TEST(Pool, PushAndGet) {

    nbl::Pool<i32> pool;
    ASSERT_TRUE(nbl::Pool<i32>::create(256, &pool).success());

    i32* value1;
    ASSERT_EQ(pool.push(&value1), nbl::Result::SUCCESS);
    *value1 = 42;

    i32* value2;
    ASSERT_EQ(pool.push(&value2), nbl::Result::SUCCESS);
    *value2 = 84;

    const i32* outPtr = pool.get(0);
    ASSERT_NE(outPtr, nullptr);
    ASSERT_EQ(*outPtr, 42);

    outPtr = pool.get(1);
    ASSERT_NE(outPtr, nullptr);
    ASSERT_EQ(*outPtr, 84);
}

TEST(Pool, OutOfBounds) {


    nbl::Pool<i32> pool{};
    ASSERT_TRUE(nbl::Pool<i32>::create(256, &pool).success());

    ASSERT_EQ(pool.get(0), nullptr);
}

TEST(Pool, Iterate) {

    nbl::Pool<i32> pool;
    ASSERT_TRUE(nbl::Pool<i32>::create(256, &pool).success());

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
    for(i32 value : pool) {
        ASSERT_TRUE(value == *value1 || value == *value2 || value == *value3);
        index++;
    }

    ASSERT_EQ(index, 3);

}

TEST(Pool, PopAndIterateStart) {

    nbl::Pool<i32> pool;
    ASSERT_TRUE(nbl::Pool<i32>::create(256, &pool).success());

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
    for(i32 value : pool) {
        ASSERT_TRUE(value == *value2 || value == *value3);
        ASSERT_FALSE(value == *value1);
        index++;
    }

    ASSERT_EQ(index, 2);

}


TEST(Pool, PopAndIterateMiddle) {
    nbl::Pool<i32> pool;
    ASSERT_TRUE(nbl::Pool<i32>::create(256, &pool).success());

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
    for(i32 value : pool) {
        ASSERT_TRUE(value == *value1 || value == *value3);
        ASSERT_FALSE(value == *value2);
        index++;
    }

    ASSERT_EQ(index, 2);

}


TEST(Pool, PopAndIterateEnd) {

    nbl::Pool<i32> pool;
    ASSERT_TRUE(nbl::Pool<i32>::create(256, &pool).success());

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
    for(i32 value : pool) {
        ASSERT_TRUE(value == *value1 || value == *value2);
        ASSERT_FALSE(value == *value3);
        index++;
    }

    ASSERT_EQ(index, 2);

}
