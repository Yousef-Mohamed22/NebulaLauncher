// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Assert.cpp
 * \author jwall64
 * \brief Implementation for Platform/Assert.hpp
 */

#include <Platform/Assert.hpp>

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
