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


/**
 * \file Core/Random.hpp
 * \author jwall64
 * \brief Pseudo-random number generator
 */

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
