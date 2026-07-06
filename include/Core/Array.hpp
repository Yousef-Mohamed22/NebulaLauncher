// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Array.hpp
 * \author jwall64
 * \brief Fixed length array class.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Span.hpp>

namespace nbl {

/**
 * \brief A simple implementation of a fixed-size array type that can be initialized with a list of values.
 * \tparam T The type of the elements in the array.
 * \tparam Length The number of elements in the array.
 */
template<typename T, usize Length>
struct Array {

    T data[Length];

    /**
     * \brief Returns the size (length) of the array
     * \return The length of the array
     */
    constexpr usize size() const { return Length; }

    /**
     * \brief Index operator to access elements in the array.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    constexpr const T& operator[](usize index) const { return data[index]; }

    /**
     * \brief Index operator to access elements in the array.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    constexpr T& operator[](usize index) { return data[index]; }

    /** 
     * \brief Implicit conversion operator that allows an Array to be used as a Span. This operator constructs a Span struct with the appropriate data pointer and length for the array.
     * \return A Span representing the elements in the array.
     */
    constexpr operator Span<T>() {
        return Span<T>{ data, Length };
    }

    constexpr operator Span<const T>() const {
        return Span<const T>{ data, Length };
    }

    /** 
     * \brief Returns an iterator to the first element in the array.
     * \return A pointer to the first element in the array.
     */
    constexpr T* begin() { return data; }

    /** 
     * \brief Returns an iterator to one past the last element in the array.
     * \return A pointer to one past the last element in the array.
     */
    constexpr T* end() { return data + Length; }

    /** 
     * \brief Returns a const iterator to the first element in the array.
     * \return A pointer to the first element in the array.
     */
    constexpr const T* begin() const { return data; }

    /** 
     * \brief Returns a const iterator to one past the last element in the array.
     * \return A pointer to one past the last element in the array.
     */
    constexpr const T* end() const { return data + Length; }

    /**
     * \brief Determines if the given value is present in the array.
     * \return True if the given value is present in the array.
     */
    constexpr bool contains(const T& value) const { 
        return operator Span<const T>().contains(value); 
    }
};

template<typename T, typename... Rest>
Array(T, Rest...) -> Array<T, 1 + sizeof...(Rest)>;

}
