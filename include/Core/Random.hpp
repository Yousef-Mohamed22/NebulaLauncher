#pragma once

#include <Core/Types.hpp>

namespace nbl { 

class Random {
public:

    u64 next();
    void seed(u64 seed);

private:

    u64 _mState;
};


}
