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

    /**
     * \brief Static member representing a successful result.
     */
    static Result SUCCESS;
};

inline Result Result::SUCCESS = Result(0);


namespace result {
    /**
     * \brief Enumeration of specific result values that can be used to indicate non-error results.
     */
    enum {
        END_OF_FILE = 1
    };
}

namespace error {
    /**
     * \brief Enumeration of specific error codes that can be used to indicate different error conditions.
     */
    enum {
        ILLEGAL_ARGUMENT = -1,
        ILLEGAL_STATE = -2,
        OUT_OF_BOUNDS = -3,
        NOT_FOUND = -4,
        OUT_OF_MEMORY = -5,
        IO_ERROR = -6,
        UNEXPECTED_EOF = -7,
        UNEXPECTED_TOKEN = -8,
        THREAD_ERROR = -9,
        PROCESS_ERROR = -10,
        PLUGIN_ERROR = -11,
        OUT_OF_SPACE = -12,
        UNIMPLEMENTED = -998,
        UNEXPECTED_ERROR = -999
    };
}


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
