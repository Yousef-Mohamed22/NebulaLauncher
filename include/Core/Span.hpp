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
#include <Core/Math.hpp>


namespace nbl {

template<typename T>
class Span {
public:

    constexpr Span() noexcept = default;
    constexpr Span(const Span& other) noexcept = default;

    constexpr Span(T* data, usize length) noexcept :
        _pData(data),
        _mLength(length)
    { }

    constexpr Span& operator=(const Span&) noexcept = default;

    constexpr operator Span<const T>() const noexcept {
        return Span<const T>(_pData, _mLength);
    }

    constexpr T& operator[](usize index) noexcept {
        NBL_ASSERT(index < _mLength);
        return _pData[index];
    }

    constexpr const T& operator[](usize index) const noexcept {
        NBL_ASSERT(index < _mLength);
        return _pData[index];
    }

    constexpr T* data() noexcept { return _pData; }
    constexpr const T* data() const noexcept { return _pData; }
    constexpr usize length() const noexcept { return _mLength; }

    constexpr T* begin() noexcept { return _pData; }
    constexpr T* end() noexcept { return _pData + _mLength; }

    constexpr const T* begin() const noexcept { return _pData; }
    constexpr const T* end() const noexcept { return _pData + _mLength; }

    constexpr Span slice(usize start) const noexcept {
        if(start > this->_mLength) return {};
        return Span(_pData + start, _mLength - start);
    }

    constexpr Span slice(usize start, usize length) const noexcept {
        if(start > this->_mLength) return {};
        return Span(_pData + start, min(length, _mLength - start));
    }
    
    constexpr Span head(usize amount) const noexcept {
        if(amount >= _mLength) return *this;
        return Span(_pData, amount);
    }

    constexpr Span tail(usize amount) const noexcept {
        if(amount >= _mLength) return *this;
        return Span(_pData + _mLength - amount, _mLength - amount);
    }

    constexpr bool contains(const T& value) const noexcept {
        for(usize i = 0 ; i < _mLength ; i++) {
            if(_pData[i] == value) return true;
        }
        return false;
    }

    template<typename R>
    constexpr Span<R> reinterpret() {
        usize maxLength = SIZE_MAX / sizeof(T);
        NBL_ASSERT(_mLength < maxLength);
        usize lengthInBytes = _mLength * sizeof(T);
        usize newLength = lengthInBytes / sizeof(R);
        return Span<R>(reinterpret_cast<R*>(_pData), newLength);
    }

private:
    T* _pData;
    usize _mLength;
};

template<typename T>
using ConstSpan = Span<const T>;


}
