// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Mutex.cpp
 * \author jwall64
 * \brief Implementation of Platform/Mutex.cpp for Linux
 */

#include <Platform/PlatformDefines.hpp>

#ifdef NBL_PLATFORM_LINUX

#include <Core/Result.hpp>
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
        return nbl::Result::THREAD_ERROR;
    }

    return nbl::Result::SUCCESS;
}



nbl::Condition::~Condition() {
    ::pthread_cond_broadcast(&_mHandle);
    ::pthread_cond_destroy(&_mHandle);
}

void nbl::Condition::signal() {
    ::pthread_cond_signal(&_mHandle);
}
void nbl::Condition::broadcast() {
    ::pthread_cond_broadcast(&_mHandle);
}
void nbl::Condition::wait(Mutex& mutex) {
    ::pthread_cond_wait(&_mHandle, &mutex._mHandle);
}

nbl::Result nbl::Condition::create(Condition* out) {
    int rc = ::pthread_cond_init(&out->_mHandle, nullptr);
    if(rc != 0) {
        return nbl::Result::THREAD_ERROR;
    }
    return nbl::Result::SUCCESS;
}



nbl::Latch::~Latch() {
    if(_bInitialized) {
        ::pthread_cond_broadcast(&_mCondition);
        ::pthread_cond_destroy(&_mCondition);
        ::pthread_mutex_destroy(&_mMutex);
    }   
}

void nbl::Latch::increment() {
    _mCount++;
}

void nbl::Latch::decrement() {
    if(_mCount.fetch_sub(1) == 1) {
        ::pthread_cond_broadcast(&_mCondition);
    }
}

void nbl::Latch::wait() {
    if(_mCount == 0) return;
    ::pthread_cond_wait(&_mCondition, &_mMutex);
}

nbl::Result nbl::Latch::create(usize count, Latch* out) {

    out->_bInitialized = true;
    out->_mCount = count;

    int rc = ::pthread_cond_init(&out->_mCondition, nullptr);
    if(rc != 0) {
        return nbl::Result::THREAD_ERROR;
    }

    rc = ::pthread_mutex_init(&out->_mMutex, nullptr);
    if(rc != 0) {
        return nbl::Result::THREAD_ERROR;
    }
    return nbl::Result::SUCCESS;

}


#endif 
