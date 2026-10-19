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
 * \file Core/Assert.hpp
 * \author jwall64
 * \brief Contains the NBL_ASSERT macro
 */


#pragma once

#if defined(NDEBUG)

namespace nbl {

/**
 * \brief Internal function that is called when an assertion fails. This should not be called directly.
 * \see NBL_ASSERT
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
 * \brief Internal function that is called when an assertion fails. This should not be called directly.
 * \see NBL_ASSERT
 */
void _assertionFailed(const char* message, const char* file = nullptr, int line = 0);

}

/**
 * \brief Macro for asserting that a given expression is true. If the expression evaluates to false, an error message is logged and a debug breakpoint is triggered.
 * \param expression The expression to assert, which should evaluate to a boolean value.
 */
#define NBL_ASSERT(expression) if(expression) {} else { nbl::_assertionFailed(#expression, __FILE__, __LINE__); }

#endif
