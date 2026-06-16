// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Mutex.hpp
 * \author jwall64
 * \brief Interface for thread synchronization
 */

#pragma once

#include <Platform/PlatformDefines.hpp>
#include <Core/Result.hpp>


#ifdef NBL_PLATFORM_LINUX

#include <pthread.h>

#endif


namespace nbl {


class Mutex {
public:

    Mutex() = default;
    ~Mutex();

    void lock();
    void unlock();

    static Result create(Mutex* out);

private:

#ifdef NBL_PLATFORM_LINUX

    pthread_mutex_t _mHandle;

#else 

    void* _mHandle;

#endif

};

}

