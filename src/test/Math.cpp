#include <Core/Math.hpp>
#include <Core/Types.hpp>

#include <gtest/gtest.h>

struct F2Result {
    i32 p1;
    i32 p2;
    i32 ret;
};

struct F3Result {
    i32 p1;
    i32 p2;
    i32 p3;
    i32 ret;
};

class Min : public ::testing::TestWithParam<F2Result> {};
class Max : public ::testing::TestWithParam<F2Result> {};
class AlignUp : public ::testing::TestWithParam<F2Result> {};
class Clamp : public ::testing::TestWithParam<F3Result> {};

TEST_P(Min, Param) {
    F2Result res = GetParam();
    ASSERT_EQ(nbl::min(res.p1, res.p2), res.ret);
}
TEST_P(Max, Param) {
    F2Result res = GetParam();
    ASSERT_EQ(nbl::max(res.p1, res.p2), res.ret);
}
TEST_P(Clamp, Param) {
    F3Result res = GetParam();
    ASSERT_EQ(nbl::clamp(res.p1, res.p2, res.p3), res.ret);
}
TEST_P(AlignUp, Param) {
    F2Result res = GetParam();
    ASSERT_EQ(nbl::alignUp(res.p1, res.p2), res.ret);
}

INSTANTIATE_TEST_SUITE_P(
    Math,
    Min,
    ::testing::Values(
        F2Result{ 0, 0, 0 },
        F2Result{ 0, 1, 0 },
        F2Result{ 1, 0, 0 },
        F2Result{ 1, 1, 1 },
        F2Result{ 0, -1, -1 },
        F2Result{ -1, 0, -1 },
        F2Result{ 1, -1, -1 },
        F2Result{ -1, 1, -1 },
        F2Result{ 1000, 30000, 1000 }
    )
);

INSTANTIATE_TEST_SUITE_P(
    Math,
    Max,
    ::testing::Values(
        F2Result{ 0, 0, 0 },
        F2Result{ 0, 1, 1 },
        F2Result{ 1, 0, 1 },
        F2Result{ 1, 1, 1 },
        F2Result{ 0, -1, 0 },
        F2Result{ -1, 0, 0 },
        F2Result{ 1, -1, 1 },
        F2Result{ -1, 1, 1 },
        F2Result{ 1000, 30000, 30000 }
    )
);

INSTANTIATE_TEST_SUITE_P(
    Math,
    Clamp,
    ::testing::Values(
        F3Result{ 0, 0, 0, 0 },
        F3Result{ 0, 1, 2, 1 },
        F3Result{ 1, 0, 2, 1 },
        F3Result{ 100, 1, 5, 5 },
        F3Result{ 0, -1, 0, 0 },
        F3Result{ -1, 0, 0, 0 },
        F3Result{ 1, -1, 1, 1 },
        F3Result{ -1, 1, 1, 1 },
        F3Result{ 1000, 5000, 30000, 5000 }
    )
);

INSTANTIATE_TEST_SUITE_P(
    Math,
    AlignUp,
    ::testing::Values(
        F2Result{ 0, 8, 0 },
        F2Result{ 2, 8, 8 },
        F2Result{ 60, 64, 64 }
    )
);
