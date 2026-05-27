// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Time.cpp
 * \author jwall64
 * \brief Implementation of Platform/Time.hpp for Linux.
 */

#include <Platform/PlatformDefines.hpp>

#ifdef NBL_PLATFORM_LINUX

#include <Platform/Time.hpp>

#include <time.h>

u64 nbl::getCurrentTime() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<u64>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
}

#endif
