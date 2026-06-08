// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file String.hpp
 * \author jwall64
 * \brief A simple implementation of a string view type and related functions for serializing values.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Types.hpp>
#include <Core/Stream.hpp>

#include <Platform/Assert.hpp>

#include <cstring>

namespace nbl {

/**
 * \brief A simple implementation of a string view type that represents a non-owning view into a string.
 */
struct StringView {

    /**
     * \brief The length of the string view, in bytes.
     */
    usize length;

    /**
     * \brief Pointer to the character data of the string view. May be nullptr if length is 0.
     */
    const char* data;
    
    /**
     * \brief Default constructor that creates an empty string view.
     */
    constexpr StringView() = default;

    /**
     * \brief Constructor that creates a string view from a pointer to character data and a length.
     * \param length The length of the string view, in bytes.
     * \param str Pointer to the character data of the string view.
     */
    constexpr StringView(usize length, const char* str) :
        length(length),
        data(str)
    { }

    /**
     * \brief Constructor that creates a string view from a null-terminated C-style string.
     * \param str Pointer to the null-terminated C-style string.
     */
    constexpr StringView(const char* str) :
        length(0),
        data(str)
    { 
        while(str[length] != 0) length++; 
    }

    /**
     * \brief Index operator to access characters in the string view.
     * \param index The index of the character to access.
     * \return The character at the specified index.
     */
    constexpr char operator[](usize index) const {
        NBL_ASSERT(index < length);
        return data[index]; 
    }

    /**
     * \brief Equality operator to compare two string views for equality.
     * \param other The other string view to compare with.
     * \return true if the string views are equal, false otherwise.
     */
    constexpr bool operator==(StringView other) const { 
        if(length != other.length) return false;
        return memcmp(data, other.data, length) == 0;
    }

    /**
     * \brief Inequality operator to compare two string views for inequality.
     * \param other The other string view to compare with.
     * \return true if the string views are not equal, false otherwise.
     */
    constexpr bool operator!=(StringView other) const {
        return !(*this == other);
    }

    /**
     * \brief Equality operator to compare a string view with a null-terminated C-style string for equality.
     * \param other Pointer to the null-terminated C-style string to compare with.
     * \return true if the string view is equal to the C-style string, false otherwise.
     */
    constexpr bool operator==(const char* other) const {
        return *this == StringView(other);
    }

    /**
     * \brief Inequality operator to compare a string view with a null-terminated C-style string for inequality.
     * \param other Pointer to the null-terminated C-style string to compare with.
     * \return true if the string view is not equal to the C-style string, false otherwise.
     */
    constexpr bool operator!=(const char* other) const {
        return !(*this == other);
    }

    /**
     * \brief Returns an iterator to the first character in the string view.
     * \return A pointer to the first character in the string view.
     */
    constexpr const char* begin() const {
        return data;
    }

    /**
     * \brief Returns an iterator to one past the last character in the string view.
     * \return A pointer to one past the last character in the string view.
     */
    constexpr const char* end() const {
        return data + length;
    }

    /**
     * \brief Computes a hash value for the string view using the djb2 algorithm.
     * \return The computed hash value for the string view.
     */
    constexpr usize hash() const {
        usize hash = 5381;
        for (auto i = 0; i < length; i++) {
            hash = ((hash << 5) + hash) + data[i];
        }
        return hash;
    }

    /**
     * \brief Checks if the string view starts with a given prefix.
     * \param other The prefix to check for.
     * \return true if the string view starts with the prefix, false otherwise.
     */
    constexpr bool startsWith(StringView other) const {
        if(length < other.length) return false;
        return memcmp(data, other.data, other.length) == 0;
    }

    /**
     * \brief Checks if the string view ends with a given suffix.
     * \param other The suffix to check for.
     * \return true if the string view ends with the suffix, false otherwise.
     */
    constexpr bool endsWith(StringView other) const {
        if(length < other.length) return false;
        return memcmp(other.data, data + length - other.length, other.length) == 0;
    }

    /**
     * \brief Checks if the string view is empty (i.e., has a length of 0).
     * \return true if the string view is empty, false otherwise.
     */
    constexpr bool empty() const {
        return length == 0;
    }

    constexpr StringView slice(usize start, usize length) const {
        NBL_ASSERT(start + length <= this->length);
        return StringView(length, data + start);
    }

    constexpr Result findFirst(char c, usize* outIndex) const {
        for (usize i = 0; i < length; i++) {
            if (data[i] == c) {
                if(outIndex) *outIndex = i;
            }
        }
        return nbl::error::NOT_FOUND;
    }

    constexpr Result findLast(char c, usize* outIndex) const {
        if(length == 0) return nbl::error::NOT_FOUND;
        for (usize i = length - 1; i >= 0; i--) {
            if (data[i] == c) {
                if(outIndex) *outIndex = i;
            }
        }
        return nbl::error::NOT_FOUND;
    }
};


/**
 * \brief Writes an integer value to a writer in a human-readable format.
 * \param value The integer value to write.
 * \param writer The writer to write the value to.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeInt(i64 value, Writer writer);

/**
 * \brief Writes an unsigned integer value to a writer in a human-readable format.
 * \param value The unsigned integer value to write.
 * \param writer The writer to write the value to.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeUint(u64 value, Writer writer);

/**
 * \brief Writes a floating-point value to a writer in a human-readable format with a specified precision.
 * \param value The floating-point value to write.
 * \param writer The writer to write the value to.
 * \param precision The number of decimal places to include in the output. Default is 15.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeFloat(f64 value, Writer writer, u8 precision = 15);


/**
* \brief Converts a value of any type to a string representation and writes it to a writer. This is a template function that can be specialized for different types.
* \tparam T The type of the value to convert to a string.
* \param value The value to convert to a string.
* \param writer The writer to write the string representation of the value to.
* \return A Result indicating success or failure of the write operation. By default, this function returns an error indicating that the conversion is unimplemented for the given type, and should be specialized for specific types to provide actual implementations.
*/
template<typename T>
Result toString(T value, Writer writer) {
    return nbl::error::UNIMPLEMENTED;
}


// Specializations of the toString function for specific types, providing implementations for converting those types to string representations and writing them to a writer.
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
 * \brief A simple struct that holds a floating-point value and a precision for formatting the value as a string. This can be used to specify the precision when converting a floating-point value to a string representation.
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

/**
 * \brief A simple implementation of a string reader that can read data from a StringView. This struct implements the Reader interface, allowing it to be used wherever a Reader is expected.
 */
struct StringReader {

    /** 
     * \brief The StringView that the StringReader reads from. This is a non-owning view into a string, and the StringReader does not manage the lifetime of the string data.
     */
    StringView str;

    /**
     * \brief The current position in the StringView from which the next read operation will start. This is an index into the StringView's data, and is updated as data is read from the StringView.
     */
    usize position;



    /**
     * \brief Constructor that initializes the StringReader with a StringView to read from. 
     * \param str The StringView that the StringReader will read from.
     */
    StringReader(StringView str) :
        str(str),
        position(0)
    { }

    /**
     * \brief Reads data from the StringView into a provided buffer. This function implements the read operation for the Reader interface.
     * \param data Pointer to a buffer that will receive the read data.
     * \param length The maximum number of bytes to read from the StringView.
     * \param bytesRead Pointer to a variable that will receive the number of bytes actually read. Can be nullptr.
     * \return A Result indicating success or failure of the read operation. If the end of the StringView is reached, it returns nbl::result::END_OF_FILE.
     */
    Result read(void* data, usize length, usize* bytesRead) {
        if (position >= str.length) {
            if(bytesRead) *bytesRead = 0;
            return nbl::result::END_OF_FILE;
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

    /**
     * \brief Implicit conversion operator that allows a StringReader to be used as a Reader. This operator constructs a Reader struct with the appropriate implementation for reading from the StringView.
     * \return A Reader struct that can be used to read from the StringView using the Reader interface.
     */
    operator Reader() {
        return Reader{ this, [](void* impl, void* data, usize length, usize* bytesRead) {
            return static_cast<StringReader*>(impl)->read(data, length, bytesRead);
        } };
    }
};


}
