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

nbl::Result nbl::utcFromComponents(u32 year, u8 month, u8 day, u8 hour, u8 minute, u8 second, u64* out) {

    if(out == nullptr
    || year > 9999 || year < 1970
    || month > 12 || month < 1
    || day > 31
    || hour > 23
    || minute > 59
    || second > 59) return nbl::Result::INVALID_ARGUMENT;

    struct tm t{};
    t.tm_year = year - 1900;
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min = minute;
    t.tm_sec = second;

    time_t time = timegm(&t);
    if(time == -1) {
        return nbl::Result::OS_ERROR;
    }
    *out = time * 1000;
    return nbl::Result::SUCCESS;
}

#endif
