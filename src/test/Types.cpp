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

#include <gtest/gtest.h>

TEST(Types, Size) {

    ASSERT_EQ(sizeof(u8),  1);
    ASSERT_EQ(sizeof(u16), 2);
    ASSERT_EQ(sizeof(u32), 4);
    ASSERT_EQ(sizeof(u64), 8);

    ASSERT_EQ(sizeof(i8),  1);
    ASSERT_EQ(sizeof(i16), 2);
    ASSERT_EQ(sizeof(i32), 4);
    ASSERT_EQ(sizeof(i64), 8);

    ASSERT_EQ(sizeof(f32), 4);
    ASSERT_EQ(sizeof(f64), 8);

    ASSERT_EQ(sizeof(usize), sizeof(void*));
    ASSERT_EQ(sizeof(isize), sizeof(void*));

}
