// Copyright (c) jwall64. All rights reserved.

/**
 * \file Assert.hpp
 * \author jwall64
 * \brief A simple implementation of an assertion macro that can be used to check for conditions that should always be true during development and debugging.
 */

#pragma once

#if defined(NDEBUG)

namespace nbl {

/**
 * \brief Internal function that is called when an assertion fails. This function logs an error message and either exits the program or triggers a debug breakpoint, depending on whether NDEBUG is defined. This should not be called directly; instead, the NBL_ASSERT macro should be used to perform assertions in the code.
 * \param message The message to be logged when the assertion fails.
 */
void _assertionFailed(const char* message);

}

/**
 * \brief Macro for asserting that a given expression is true. If the expression evaluates to false, an error message is logged and the program exits with exit code 255.
 * \param expression The expression to assert, which should evaluate to a boolean value. 
 */
#define NBL_ASSERT(expression) if(expression) {} else { nbl::_assertionFailed(#expression); }

#else

namespace nbl {

/**
 * \brief Internal function that is called when an assertion fails. This function logs an error message and either exits the program or triggers a debug breakpoint, depending on whether NDEBUG is defined. This should not be called directly; instead, the NBL_ASSERT macro should be used to perform assertions in the code.
 * \param message The message to be logged when the assertion fails, which typically includes the expression that failed and the file and line number where the assertion occurred.
 * \param file The name of the source file where the assertion failed. This is typically provided by the __FILE__ macro.
 * \param line The line number in the source file where the assertion failed. This is typically provided by the __LINE__ macro.
 */
void _assertionFailed(const char* message, const char* file = nullptr, int line = 0);

}

/**
 * \brief Macro for asserting that a given expression is true. If the expression evaluates to false, an error message is logged and a debug breakpoint is triggered.
 * \param expression The expression to assert, which should evaluate to a boolean value.
 */
#define NBL_ASSERT(expression) if(expression) {} else { nbl::_assertionFailed(#expression, __FILE__, __LINE__); }

#endif
