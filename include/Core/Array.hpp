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
 * \file Core/Array.hpp
 * \author jwall64
 * \brief Contains a fixed-size array type
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Span.hpp>

namespace nbl {

/**
 * \brief A simple implementation of a fixed-size array.
 * \tparam T The type of the elements in the array.
 * \tparam Length The number of elements in the array.
 */
template<typename T, usize Length>
struct Array {

    T data[Length];


    constexpr usize size() const { return Length; }

    constexpr const T& operator[](usize index) const { return data[index]; }
    constexpr T& operator[](usize index) { return data[index]; }


    constexpr operator Span<T>() {
        return Span<T> { data, Length };
    }
    constexpr operator ConstSpan<T>() const {
        return ConstSpan<T> { data, Length };
    }

    constexpr Span<T> span() {
        return Span<T> { data, Length };
    }

    constexpr ConstSpan<T> span() const {
        return ConstSpan<T> { data, Length };
    }

    constexpr ConstSpan<T> constSpan() const {
        return ConstSpan<T> { data, Length };
    }


    constexpr T* begin() { return data; }
    constexpr T* end() { return data + Length; }


    constexpr const T* begin() const { return data; }
    constexpr const T* end() const { return data + Length; }


    constexpr bool contains(const T& value) const { 
        return span().contains(value); 
    }
};

template<typename T, typename... Rest>
Array(T, Rest...) -> Array<T, 1 + sizeof...(Rest)>;

}
