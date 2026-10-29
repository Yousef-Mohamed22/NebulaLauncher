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
 * \file Core/String.hpp
 * \author jwall64
 * \brief Functions for creating, manipulating, and comparing strings.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Types.hpp>
#include <Core/Stream.hpp>

#include <Core/Assert.hpp>

#include <cstring>

namespace nbl {



class StringView {
public:
    constexpr StringView() :
        _pData(nullptr),
        _mLength(0),
        _bNullTerminated(false)
    { }

    constexpr StringView(const StringView& other) noexcept = default;
    constexpr StringView(StringView&& other) noexcept = default;

    constexpr StringView(const char* str, usize length, bool nullTerminated = false) :
        _pData(str),
        _mLength(length),
        _bNullTerminated(nullTerminated)
    { }

    constexpr StringView(const char* str) :
        _pData(str),
        _mLength(_cStringLength(str)),
        _bNullTerminated(str != nullptr)
    { }

    constexpr StringView& operator=(const StringView& other) = default;

    constexpr StringView& operator=(const char* other) {
        _pData = other;
        _mLength = _cStringLength(other);
        _bNullTerminated = true;
        return *this;
    }

    constexpr char operator[](usize index) const noexcept {
        NBL_ASSERT(index < _mLength);
        return _pData[index]; 
    }


    constexpr bool operator==(StringView other) const noexcept { 
        if(_mLength != other._mLength) return false;
        return memcmp(_pData, other._pData, _mLength) == 0;
    }

    constexpr bool operator==(const char* other) const noexcept {
        return *this == StringView(other);
    }


    constexpr bool operator!=(StringView other) const noexcept {
        return !(*this == other);
    }

    constexpr bool operator!=(const char* other) const noexcept {
        return !(*this == other);
    }


    constexpr const char* data() const noexcept { return _pData; }
    constexpr usize length() const noexcept { return _mLength; }
    constexpr bool nullTerminated() const noexcept { return _bNullTerminated; }


    constexpr const char* begin() const noexcept {
        return _pData;
    }
    constexpr const char* end() const noexcept {
        return _pData + _mLength;
    }

 
    constexpr usize hash() const noexcept {
        usize hash = 5381;
        for (auto i = 0; i < _mLength; i++) {
            hash = ((hash << 5) + hash) + _pData[i];
        }
        return hash;
    }

    constexpr bool startsWith(StringView other) const noexcept {
        if(_mLength < other._mLength) return false;
        return other.empty() || memcmp(_pData, other._pData, other._mLength) == 0;
    }

    constexpr bool endsWith(StringView other) const noexcept {
        if(_mLength < other._mLength) return false;
        return other.empty() || memcmp(other._pData, _pData + _mLength - other._mLength, other._mLength) == 0;
    }


    constexpr bool empty() const noexcept {
        return _mLength == 0;
    }

    /**
     * \brief Returns the end of the string, starting at the given index
     */
    constexpr StringView slice(usize start) const noexcept {
        NBL_ASSERT(start <= this->_mLength);
        return StringView(_pData + start, _mLength - start, _bNullTerminated);
    }

    /**
     * \brief Creates a substring with the given start position and _mLength
     */
    constexpr StringView slice(usize start, usize length) const noexcept {
        NBL_ASSERT(start + length <= _mLength);
        return StringView(_pData + start, length, start + length == _mLength ? _bNullTerminated : _pData[start + length] == '\0');
    }


    constexpr StringView head(usize amount) const noexcept {
        if(amount >= _mLength) return *this;
        return StringView(_pData, amount, _pData[amount] == '\0');
    }

    constexpr StringView tail(usize amount) const noexcept {
        if(amount >= _mLength) return *this;
        return StringView(_pData + _mLength - amount, _mLength - amount, _bNullTerminated);

    }

    constexpr StringView lstrip(char c = ' ', usize minLength = 0) const noexcept {
        usize length = _mLength;
        const char* data = _pData;
        while(length > minLength && *_pData == c) {
            data++;
            length--;
        }
        return StringView(data, length, _bNullTerminated);
    }
    constexpr StringView rstrip(char c = ' ', usize minLength = 0) const noexcept {
        usize length = _mLength;
        const char* data = _pData;
        while(length > minLength && data[length - 1] == c) {
            length--;
        }
        return StringView(data, length, _bNullTerminated);
    }

    /**
     * \brief Finds the first occurrance of a given character in the string
     */
    constexpr Result findFirst(char c, usize* outIndex, usize startIndex = 0) const noexcept {
        for (usize i = startIndex; i < _mLength; i++) {
            if (_pData[i] == c) {
                if(outIndex) *outIndex = i;
                return nbl::Result::SUCCESS;
            }
        }
        return nbl::Result::NOT_FOUND;
    }

    /**
     * \brief Finds the final occurrance of a given character in the string
     */
    constexpr Result findLast(char c, usize* outIndex) const noexcept {
        if(_mLength == 0) return nbl::Result::NOT_FOUND;
        for (usize i = _mLength - 1; i > 0; i--) {
            if (_pData[i] == c) {
                if(outIndex) *outIndex = i;
                return nbl::Result::SUCCESS;
            }
        }
        return nbl::Result::NOT_FOUND;
    }

    /**
     * \brief Computes the Levenshtein distance between this string and another string
     */
    constexpr usize distance(StringView other) const noexcept {

        if(other._mLength == 0) return _mLength;
        if(_mLength == 0) return other._mLength;

        if(_pData[0] == other._pData[0]) {
            return slice(1).distance(other.slice(1));
        }

        usize v1 = slice(1).distance(other);
        usize v2 = distance(other.slice(1));
        usize v3 = slice(1).distance(other.slice(1));

        if(v1 <= v2 && v1 <= v3) return 1 + v1;
        if(v2 <= v1 && v2 <= v3) return 1 + v2;

        return 1 + v3;
    }

private:

    const char* _pData;
    struct {
        usize _mLength : (sizeof(usize) * 8) - 1;
        usize _bNullTerminated: 1;
    };
    
    constexpr static usize _cStringLength(const char* str) {
        if(!str) return 0;
        usize out = 0;
        while(str[out] != '\0') ++out;
        return out;
    }
};



inline Result write(StringView value, Writer writer) { return writer.writeAll(value.data(), value.length()); }
inline Result write(const char* value, Writer writer) { return writer.writeAll(value, strlen(value)); }





/**
 * \brief A Reader that can read data from a StringView. This struct implements the Reader interface, allowing it to be used wherever a Reader is expected.
 */
struct StringReader {

    StringView str;
    usize position;

    StringReader(StringView str) :
        str(str),
        position(0)
    { }

    Result read(void* data, usize length, usize* bytesRead) {
        if (position >= str.length()) {
            if(bytesRead) *bytesRead = 0;
            return nbl::Result::END_OF_STREAM;
        }

        usize remaining = str.length() - position;
        usize toRead = length < remaining ? length : remaining;
        memcpy(data, str.data() + position, toRead);
        position += toRead;
        if(bytesRead) {
            *bytesRead = toRead;
        }
        return Result::SUCCESS;
    }

    operator Reader() {
        return Reader{ this, [](void* impl, void* data, usize length, usize* bytesRead) {
            return static_cast<StringReader*>(impl)->read(data, length, bytesRead);
        } };
    }
};


}
