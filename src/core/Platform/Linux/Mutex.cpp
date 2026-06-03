// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Mutex.cpp
 * \author jwall64
 * \brief Implementation of Platform/Mutex.cpp for Linux
 */

#include "Core/Result.hpp"
#include <Platform/PlatformDefines.hpp>

#ifdef NBL_PLATFORM_LINUX

#include <Platform/Mutex.hpp>

#include <pthread.h>

nbl::Mutex::~Mutex() {
    ::pthread_mutex_destroy(&_mHandle);
}

void nbl::Mutex::lock() {
    ::pthread_mutex_lock(&_mHandle);
}


void nbl::Mutex::unlock() {
    ::pthread_mutex_unlock(&_mHandle);
}

nbl::Result nbl::Mutex::create(Mutex* out) {

    int rc = ::pthread_mutex_init(&out->_mHandle, nullptr);
    if(rc != 0) {
        return nbl::error::THREAD_ERROR;
    }

    return nbl::Result::SUCCESS;
}


#endif 
