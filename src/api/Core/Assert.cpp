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
 * \file Core/Assert.cpp
 * \author jwall64
 * \brief Implementation of Core/Assert.hpp
 */

#include <Core/Assert.hpp>

#include <Core/String.hpp>
#include <Platform/Logging.hpp>
#include <Platform/PlatformDefines.hpp>


#if defined(NDEBUG)

#include <stdlib.h>

void nbl::_assertionFailed(const char* message) {
    nbl::log(nbl::LogLevel::FATAL, "Assertion failed: ", message);
    exit(255);
}

#else 

void nbl::_assertionFailed(const char* message, const char* file, int line) {
    nbl::log(nbl::LogLevel::FATAL, "Assertion failed: ", message, " (", file, ":", line, ")");
    NBL_DEBUG_BREAK();
}

#endif
