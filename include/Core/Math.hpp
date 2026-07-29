// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Math.hpp
 * \author jwall64
 * \brief Common mathematical utility functions.
 */

#pragma once

namespace nbl {


/**
 * \brief Returns the smaller of two values.
 * \param v1 First value.
 * \param v2 Second value.
 * \return The smaller of the two values.
 */
template<typename T>
T min(T v1, T v2) {
    return v1 > v2 ? v2 : v1;
}

/**
 * \brief Returns the larger of two values.
 * \param v1 First value.
 * \param v2 Second value.
 * \return The larger of the two values.
 */
template<typename T>
T max(T v1, T v2) {
    return v1 < v2 ? v2 : v1;
}

/**
 * \brief Clamps a value between a minimum and maximum bound.
 * \param value The value to clamp.
 * \param minBound The minimum bound.
 * \param maxBound The maximum bound.
 * \return The clamped value.
 */
template<typename T>
T clamp(T value, T minBound, T maxBound) {
    if(value < minBound) return minBound;
    if(value > maxBound) return maxBound;
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
