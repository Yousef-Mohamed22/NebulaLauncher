// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Span.hpp
 * \author jwall64
 * \brief A simple implementation of a span type
 */

#pragma once

#include <Core/Types.hpp>
#include <Platform/Assert.hpp>

namespace nbl {

/**
 * \brief A simple implementation of a span type that represents a contiguous sequence of elements.
 * \tparam T The type of the elements in the span.
 */
template<typename T>
struct ConstSpan {
    
    /**
     * \brief Pointer to the first element in the span.
     */
    const T* data;

    /**
     * \brief The number of elements in the span.
     */
    usize length;

    /**
     * \brief Index operator to access elements in the span.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    constexpr const T& operator[](usize index) const { 
        NBL_ASSERT(index < length);
        return data[index]; 
    }

    /**
     * \brief Returns an iterator to the first element in the span.
     * \return A pointer to the first element in the span.
     */
    constexpr const T* begin() const { return data; }

    /**
     * \brief Returns an iterator to one past the last element in the span.
     * \return A pointer to one past the last element in the span.
     */
    constexpr const T* end() const { return data + length; }

    constexpr bool contains(const T& value) const {
        for(const T& v : *this) {
            if(v == value) return true;
        }
        return false;
    }

};



/**
 * \brief A simple implementation of a span type that represents a contiguous sequence of elements.
 * \tparam T The type of the elements in the span.
 */
template<typename T>
struct Span {
    
    /**
     * \brief Pointer to the first element in the span.
     */
    T* data;

    /**
     * \brief The number of elements in the span.
     */
    usize length;

    /**
     * \brief Index operator to access elements in the span.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    constexpr const T& operator[](usize index) const { 
        NBL_ASSERT(index < length);
        return data[index]; 
    }

    /**
     * \brief Index operator to access elements in the span.
     * \param index The index of the element to access.
     * \return A reference to the element at the specified index.
     */
    constexpr T& operator[](usize index) { 
        NBL_ASSERT(index < length);
        return data[index]; 
    }

    /**
     * \brief Returns an iterator to the first element in the span.
     * \return A pointer to the first element in the span.
     */
    constexpr T* begin() { return data; }

    /**
     * \brief Returns an iterator to one past the last element in the span.
     * \return A pointer to one past the last element in the span.
     */
    constexpr T* end() { return data + length; }

    /**
     * \brief Returns a const iterator to the first element in the span.
     * \return A pointer to the first element in the span.
     */
    constexpr const T* begin() const { return data; }

    /**
     * \brief Returns a const iterator to one past the last element in the span.
     * \return A pointer to one past the last element in the span.
     */
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

};

}
