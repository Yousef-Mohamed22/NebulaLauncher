// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Array.hpp
 * \author jwall64
 * \brief Fixed length array class.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Span.hpp>

#include <initializer_list>

namespace nbl {

/**
 * \brief A simple implementation of a fixed-size array type that can be initialized with a list of values.
 * \tparam T The type of the elements in the array.
 * \tparam Length The number of elements in the array.
 */
template<typename T, usize Length>
class Array {
public:

    /** 
     * \brief Default constructor that intializes the array with default-constructed values of type T.
     */
    Array() = default;

    /**
     * \brief Constructor that initializes the array with a list of values. The constructor will copy values from the initializer list into the array until either the end of the list is reached or the array is full, whichever comes first.
     * \param values The initializer list of values to initialize the array with.
     */
    Array(const std::initializer_list<T>& values) {
        usize i = 0;
        for (const T& value : values) {
            if (i >= Length) break;
            _mValues[i++] = value;
        }
    }

    /**
     * \brief Index operator to access elements in the array.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    const T& operator[](usize index) const { return _mValues[index]; }

    /**
     * \brief Index operator to access elements in the array.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    T& operator[](usize index) { return _mValues[index]; }

    /** 
     * \brief Implicit conversion operator that allows an Array to be used as a Span. This operator constructs a Span struct with the appropriate data pointer and length for the array.
     * \return A Span representing the elements in the array.
     */
    operator Span<T>() {
        return Span<T>{ _mValues, Length };
    }

    /** 
     * \brief Returns an iterator to the first element in the array.
     * \return A pointer to the first element in the array.
     */
    T* begin() { return _mValues; }

    /** 
     * \brief Returns an iterator to one past the last element in the array.
     * \return A pointer to one past the last element in the array.
     */
    T* end() { return _mValues + Length; }

    /** 
     * \brief Returns a const iterator to the first element in the array.
     * \return A pointer to the first element in the array.
     */
    const T* begin() const { return _mValues; }

    /** 
     * \brief Returns a const iterator to one past the last element in the array.
     * \return A pointer to one past the last element in the array.
     */
    const T* end() const { return _mValues + Length; }

private:
    T _mValues[Length];
};

}
