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
 * \file Core/Span.hpp
 * \author jwall64
 * \brief Contains Span and ConstSpan types, representing non-owning views into contiguous memory.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Assert.hpp>

namespace nbl {


template<typename T>
struct ConstSpan {
    

    const T* data;
    usize length;


    constexpr const T& operator[](usize index) const { 
        NBL_ASSERT(index < length);
        return data[index]; 
    }


    constexpr const T* begin() const { return data; }
    constexpr const T* end() const { return data + length; }

    constexpr bool contains(const T& value) const {
        for(const T& v : *this) {
            if(v == value) return true;
        }
        return false;
    }

    constexpr ConstSpan<T> slice(usize start, usize length) const {
        NBL_ASSERT(start + length <= this->length);
        return { data + start, length };
    }

    constexpr ConstSpan<T> slice(usize start) const {
        NBL_ASSERT(start <= length);
        return { data + start, length - start };
    }

};



template<typename T>
struct Span {
    

    T* data;
    usize length;


    constexpr const T& operator[](usize index) const { 
        NBL_ASSERT(index < length);
        return data[index]; 
    }

    constexpr T& operator[](usize index) { 
        NBL_ASSERT(index < length);
        return data[index]; 
    }


    constexpr T* begin() { return data; }
    constexpr T* end() { return data + length; }

    constexpr const T* begin() const { return data; }
    constexpr const T* end() const { return data + length; }

    constexpr operator ConstSpan<T>() const {
        return { .data = data, .length = length };
    }

    constexpr bool contains(const T& value) const {
        for(const T& v : *this) {
            if(v == value) return true;
        }
        return false;
    }

    constexpr Span<T> slice(usize start, usize length) const {
        NBL_ASSERT(start + length <= this->length);
        return { data + start, length };
    }

};

}
