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

constexpr usize cStringLength(const char* str) {
    if(!str) return 0;
    usize out = 0;
    while(str[out] != '\0') ++out;
    return out;
}

struct StringView {

    const char* data;
    struct {
        usize length : (sizeof(usize) * 8) - 1;
        usize nullTerminated: 1;
    };
    

    constexpr StringView() = default;

    constexpr StringView(const StringView& other) noexcept = default;
    constexpr StringView(StringView&& other) noexcept = default;

    constexpr StringView(const char* str, usize length, bool nullTerminated = false) :
        data(str),
        nullTerminated(nullTerminated),
        length(length)
    { }

    constexpr StringView(const char* str) :
        data(str),
        nullTerminated(str != nullptr),
        length(cStringLength(str))
    { }

    constexpr StringView& operator=(const StringView& other) noexcept = default;
    constexpr StringView& operator=(StringView&& other) noexcept = default;

    constexpr StringView& operator=(const char* cstring) noexcept {
        data = cstring;
        length = 0;
        if(cstring) {
            while(*cstring++) length++;
        }
        return *this;
    }


    constexpr char operator[](usize index) const noexcept {
        NBL_ASSERT(index < length);
        return data[index]; 
    }


    constexpr bool operator==(StringView other) const noexcept { 
        if(length != other.length) return false;
        return memcmp(data, other.data, length) == 0;
    }

    constexpr bool operator!=(StringView other) const noexcept {
        return !(*this == other);
    }

    constexpr bool operator==(const char* other) const noexcept {
        return *this == StringView(other);
    }

    constexpr bool operator!=(const char* other) const noexcept {
        return !(*this == other);
    }


    constexpr const char* begin() const noexcept {
        return data;
    }
    constexpr const char* end() const noexcept {
        return data + length;
    }

 
    constexpr usize hash() const noexcept {
        usize hash = 5381;
        for (auto i = 0; i < length; i++) {
            hash = ((hash << 5) + hash) + data[i];
        }
        return hash;
    }

    constexpr bool startsWith(StringView other) const noexcept {
        if(length < other.length) return false;
        return other.empty() || memcmp(data, other.data, other.length) == 0;
    }

    constexpr bool endsWith(StringView other) const noexcept {
        if(length < other.length) return false;
        return other.empty() || memcmp(other.data, data + length - other.length, other.length) == 0;
    }


    constexpr bool empty() const noexcept {
        return length == 0;
    }

    /**
     * \brief Returns the end of the string, starting at the given index
     */
    constexpr StringView tail(usize start) const noexcept {
        NBL_ASSERT(start <= this->length);
        return StringView(data + start, length - start, nullTerminated);
    }

    /**
     * \brief Creates a substring with the given start position and length
     */
    constexpr StringView slice(usize start, usize length) const noexcept {
        NBL_ASSERT(start + length <= this->length);
        return StringView(data + start, length, start + length == this->length ? nullTerminated : data[start+length] == '\0');
    }

    constexpr StringView lstrip(char c = ' ', usize minLength = 0) const noexcept {
        StringView out = *this;
        while(out.length > minLength && *out.data == c) {
            out.data++;
            out.length--;
        }
        return out;
    }
    constexpr StringView rstrip(char c = ' ', usize minLength = 0) const noexcept {
        StringView out = *this;
        while(out.length > minLength && out.data[out.length - 1] == c) {
            out.length--;
        }
        return out;
    }

    /**
     * \brief Finds the first occurrance of a given character in the string
     */
    constexpr Result findFirst(char c, usize* outIndex, usize startIndex = 0) const noexcept {
        for (usize i = startIndex; i < length; i++) {
            if (data[i] == c) {
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
        if(length == 0) return nbl::Result::NOT_FOUND;
        for (usize i = length - 1; i > 0; i--) {
            if (data[i] == c) {
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

        if(other.length == 0) return length;
        if(length == 0) return other.length;

        if(data[0] == other.data[0]) {
            return tail(1).distance(other.tail(1));
        }

        usize v1 = tail(1).distance(other);
        usize v2 = distance(other.tail(1));
        usize v3 = tail(1).distance(other.tail(1));

        if(v1 <= v2 && v1 <= v3) return 1 + v1;
        if(v2 <= v1 && v2 <= v3) return 1 + v2;

        return 1 + v3;
    }
};


/**
 * \brief Writes an integer value to a writer in Base-10 ASCII.
 * \param value The integer value to write.
 * \param writer The writer to write the value to.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeInt(i64 value, Writer writer);

/**
 * \brief Writes an unsigned integer value to a writer in Base-10 ASCII.
 * \param value The unsigned integer value to write.
 * \param writer The writer to write the value to.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeUint(u64 value, Writer writer);

/**
 * \brief Writes a floating-point value to a writer in Base-10 ASCII format with a specified precision.
 * \param value The floating-point value to write.
 * \param writer The writer to write the value to.
 * \param precision The number of decimal places to include in the output. Default is 15.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeFloat(f64 value, Writer writer, u8 precision = 15);

/**
 * \brief Writes an unsigned integer value to a writer in Base-16 ASCII.
 * \param value The unsigned integer value to write.
 * \param writer The writer to write the value to.
 * \param minDigits The minimum number of digits to write. The output will be left-padded with zeros if necessary.
 * \param lower Whether to use lowercase hex digits for place values greater than 9
 * \return A Result indicating success or failure of the write operation.
 */
Result writeHex(u64 value, Writer writer, u8 minDigits = 0, bool lower = false);


/**
* \brief Converts a value of any type to a string representation and writes it to a writer. This is a template function that should be specialized for different types.
* \tparam T The type of the value to convert to a string.
* \param value The value to convert to a string.
* \param writer The writer to write the string representation of the value to.
* \return A Result indicating success or failure of the write operation. By default, this function returns an error indicating that the conversion is unimplemented for the given type, and should be specialized for specific types to provide actual implementations.
*/
template<typename T>
Result toString(T value, Writer writer) {
    return nbl::Result::UNIMPLEMENTED;
}

template<typename T>
inline Result toString(T* ptr, Writer writer) {
    NBL_CHECK(writer.writeAll("0x", 2));
    return writeHex(reinterpret_cast<u64>(ptr), writer, sizeof(T*) * 2);
}

inline Result toString(const char* value, Writer writer) { return writer.writeAll(value, strlen(value)); }
inline Result toString(StringView value, Writer writer) { return writer.writeAll(value.data, value.length); }

inline Result toString(bool value, Writer writer) { return toString(value ? "true" : "false", writer); }
inline Result toString(char value, Writer writer) { return writer.writeAll(&value, 1); }

inline Result toString(i64 value, Writer writer) { return writeInt(value, writer); }
inline Result toString(u64 value, Writer writer) { return writeUint(value, writer); }
inline Result toString(f64 value, Writer writer) { return writeFloat(value, writer); }

inline Result toString(i32 value, Writer writer) { return writeInt(static_cast<i64>(value), writer); }
inline Result toString(u32 value, Writer writer) { return writeUint(static_cast<u64>(value), writer); }
inline Result toString(f32 value, Writer writer) { return writeFloat(static_cast<f64>(value), writer); }

inline Result toString(i16 value, Writer writer) { return writeInt(static_cast<i64>(value), writer); }
inline Result toString(u16 value, Writer writer) { return writeUint(static_cast<u64>(value), writer); }

inline Result toString(i8 value, Writer writer) { return writeInt(static_cast<i64>(value), writer); }
inline Result toString(u8 value, Writer writer) { return writeUint(static_cast<u64>(value), writer); }


inline Result toString(nbl::Result value, Writer writer) {
    return toString(static_cast<i32>(value), writer);
}


/**
 * \brief A struct that holds a floating-point value and a precision for formatting the value as a string. This can be used to specify the precision when converting a floating-point value to a string representation.
 */
struct FloatLimiter {
    
    /** 
     * \brief Constructor that initializes the FloatLimiter with a floating-point value and a precision.
     * \param value The floating-point value to be formatted as a string.
     * \param precision The number of decimal places to include in the string representation of the value.
     */
    FloatLimiter(f64 value, u8 precision) : 
        value(value),
        precision(precision) 
    { }
    
    f64 value;
    u8 precision;
};

// Specialization of the toString function for the FloatLimiter struct, which writes the floating-point value to the writer with the specified precision.
inline Result toString(FloatLimiter limiter, Writer writer) {
    return writeFloat(limiter.value, writer, limiter.precision);
}


struct HexEncoder {
    ConstSpan<u8> data;
    bool upper;
};
// Specialization of the toString function for the HexEncoder struct, which writes the buffer to the stream in hexadecimal format.
Result toString(HexEncoder encoder, Writer writer);

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
        if (position >= str.length) {
            if(bytesRead) *bytesRead = 0;
            return nbl::Result::END_OF_STREAM;
        }

        usize remaining = str.length - position;
        usize toRead = length < remaining ? length : remaining;
        memcpy(data, str.data + position, toRead);
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
