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
 * \file Core/Random.cpp
 * \author jwall64
 * \brief Implementation of Core/Random.hpp
 */

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
