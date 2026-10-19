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
 * \file Core/Math.hpp
 * \author jwall64
 * \brief Common mathematical utility functions.
 */


#pragma once

namespace nbl {


template<typename T, typename O>
constexpr T min(T v1, O v2) {
    return v1 > static_cast<T>(v2) ? static_cast<T>(v2) : v1;
}


template<typename T, typename O>
constexpr T max(T v1, O v2) {
    return v1 < static_cast<T>(v2) ? static_cast<T>(v2) : v1;
}


template<typename T, typename L, typename U>
constexpr T clamp(T value, L minBound, U maxBound) {
    if(value < static_cast<T>(minBound)) return static_cast<T>(minBound);
    if(value > static_cast<T>(maxBound)) return static_cast<T>(maxBound);
    return value;
}


template<typename T, typename A>
constexpr T alignDown(T value, A alignment) noexcept {
    return (value / static_cast<T>(alignment)) * static_cast<T>(alignment);
}

template<typename T, typename A>
constexpr T alignUp(T value, A alignment) noexcept {
    return ((value + static_cast<T>(alignment) - 1) / static_cast<T>(alignment)) * static_cast<T>(alignment);
}


}
