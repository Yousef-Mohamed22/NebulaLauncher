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
