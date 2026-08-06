// Copyright (C) 2026 jwall64. All rights reserved.

/**
 * \file Optional.hpp
 * \author jwall64
 * \brief A simple implementation of an optional type.
 */

#pragma once

#include <Core/Types.hpp>
#include <Platform/Assert.hpp>

#include <utility>
#include <new>

namespace nbl {

/**
 * \brief A simple implementation of an optional type that can hold a value of type T or be empty.
 * \tparam T The type of the value that can be held by the optional.
 */
template<typename T>
class Optional {
public:

    /**
     * \brief Default constructor that creates an empty optional.
     */
    Optional() :
        _mHasValue(false)
    { }

    /**
     * \brief Constructor that creates a filled optional by copying a value.
     * \param value The value to be held by the optional.
     */
    Optional(const T& value) :
        _mHasValue(true)
    {
        new (_mValue) T(value);
    }
    
    /**
     * \brief Constructor that creates a filled optional by moving a value.
     * \param value The value to be held by the optional.
     */
    Optional(T&& value) :
        _mHasValue(true)
    {
        new (_mValue) T(std::move(value));
    }

    /**
     * \brief Copy constructor that creates a new optional by copying another optional.
     * \param other The optional to copy from.
     */
    Optional(const Optional& other) :
        _mHasValue(other._mHasValue)
    {
        if(_mHasValue) {
            new (_mValue) T(*reinterpret_cast<const T*>(other._mValue));
        }
    }

    /**
     * \brief Move constructor that creates a new optional by moving another optional.
     * \param other The optional to move from.
     */
    Optional(Optional&& other) :
        _mHasValue(other._mHasValue)
    {
        if(_mHasValue) {
            new (_mValue) T(std::move(*reinterpret_cast<T*>(other._mValue)));
            other._mHasValue = false;
        }
    }

    /**
     * \brief Destructor that destroys the value held by the optional if it exists.
     */
    ~Optional() {
        if(_mHasValue) {
            reinterpret_cast<T*>(_mValue)->~T();
        }
    }

    /**
     * \brief Copy assignment operator that creates a new optional by copying from another optional.
     * \param other The optional to move from.
     * \return A reference to the optional after assignment.
     */
    Optional& operator=(const Optional& other) {
        if(this == &other) return *this;
        ~Optional();

        _mHasValue = other._mHasValue;
        if(_mHasValue) {
            new (_mValue) T(*reinterpret_cast<const T*>(other._mValue));
        }

        return *this;
    }

    /**
     * \brief Move assignment operator that creates a new optional by moving from another optional.
     * \param other The optional to move from.
     * \return A reference to the optional after assignment.
     */
    Optional& operator=(Optional&& other) {
        if(this == &other) return *this;
        ~Optional();

        _mHasValue = other._mHasValue;
        if(_mHasValue) {
            new (_mValue) T(std::move(*reinterpret_cast<T*>(other._mValue)));
            other._mHasValue = false;
        }

        return *this;
    }

    /**
     * \brief Assignment operator that assigns a value to the optional.
     * \param value The value to be assigned to the optional.
     * \return A reference to the optional after assignment.
     */
    Optional& operator=(const T& value) {
        ~Optional();
        _mHasValue = true;
        new (_mValue) T(value);
        return *this;
    }
    
    /**
     * \brief Move assignment operator that assigns a value to the optional.
     * \param value The value to be assigned to the optional.
     * \return A reference to the optional after assignment.
     */
    Optional& operator=(T&& value) {
        ~Optional();
        _mHasValue = true;
        new (_mValue) T(std::move(value));
        return *this;
    }

    /**
     * \brief Checks if the optional has a value.
     * \return true if the optional has a value, false otherwise.
     */
    bool hasValue() const {
        return _mHasValue;
    }

    /**
     * \brief Dereference operator that returns a reference to the value held by the optional.
     * \return A reference to the value held by the optional.
     */
    const T& operator*() const {
        NBL_ASSERT(_mHasValue);
        return *reinterpret_cast<const T*>(_mValue);
    }

    /**
     * \brief Arrow operator that returns a pointer to the value held by the optional.
     * \return A pointer to the value held by the optional.
     */
    const T* operator->() const {
        NBL_ASSERT(_mHasValue);
        return reinterpret_cast<const T*>(_mValue);
    }

private:
    bool _mHasValue;
    u8 _mValue[sizeof(T)];
};

}
