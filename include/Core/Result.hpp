// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Result.hpp
 * \author jwall64
 * \brief A simple implementation of a result type that can represent success or various error conditions.
 */

#pragma once

#include <Core/Types.hpp>


namespace nbl {

/**
 * \brief A simple implementation of a result type that can represent success or various error conditions.
 */
struct Result {
    i32 value;

    constexpr Result() : value(0) {}

    /**
     * \brief Constructor that creates a result with a specific value.
     * \param value The value representing the result, where 0 indicates success and negative values indicate different error conditions.
     */
    constexpr Result(i32 value) :
        value(value)
    { }

    /**
     * \brief Implicit conversion operator to allow a Result to be used as an i32 value.
     * \return The integer value of the result.
     */
    constexpr operator i32() const {
        return value;
    }

    /**
     * \brief Checks if the result indicates success.
     * \return true if the result is successful (value is 0), false otherwise.
     */
    constexpr bool success() const {
        return value == 0;
    }

    /**
     * \brief Checks if the result indicates an error.
     * \return true if the result is an error (value is negative), false otherwise.
     */
    constexpr bool error() const {
        return value < 0;
    }

    enum : i32 {
        SUCCESS = 0,

        END_OF_STREAM = 1,
        CANCELED = 2,

        INVALID_ARGUMENT = -1,
        INVALID_STATE = -2,
        OUT_OF_BOUNDS = -3,
        NOT_FOUND = -4,
        OUT_OF_MEMORY = -5,
        IO_ERROR = -6,
        UNEXPECTED_END_OF_STREAM = -7,
        UNEXPECTED_TOKEN = -8,
        THREAD_ERROR = -9,
        PROCESS_ERROR = -10,
        PLUGIN_ERROR = -11,
        OUT_OF_SPACE = -12,
        PARSE_ERROR = -13,
        TIMEOUT = -14,
        OS_ERROR = -15,
        FORMAT_ERROR = -16,
        INTERRUPTED = -17,
        LIBRARY_ERROR = -18,
        TOO_MANY_OPEN_FILES = -19,
        UNIMPLEMENTED = -998,
        UNEXPECTED_ERROR = -999
    };

};



#ifndef NDEBUG 
void _reportCheckFailed(nbl::Result res, const char* file, i32 line);
#else
inline void _reportCheckFailed(nbl::Result, const char*, i32) {}
#endif

/**
 * \brief Macro to check the result of an expression and return if it indicates an error.
 * \param expr The expression to check, which should evaluate to a nbl::Result.
 */
#define NBL_CHECK(expr) { \
    nbl::Result _result = (expr); \
    if(_result.error()) { \
        nbl::_reportCheckFailed(_result.value, __FILE__, __LINE__); \
        return _result; \
    } \
}

}
