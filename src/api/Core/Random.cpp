#include <Core/Random.hpp>

u64 nbl::Random::next() {
    static constexpr u64 a = 2862933555777941757ULL;
    static constexpr u64 c = 3037000493ULL;
    _mState = (a * _mState + c);
    return _mState;
}

void nbl::Random::seed(u64 seed) {
    _mState = seed;
}
