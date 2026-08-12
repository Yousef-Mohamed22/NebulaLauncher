// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Mutex.hpp
 * \author jwall64
 * \brief Interface for thread synchronization
 */

#pragma once

#include <Platform/PlatformDefines.hpp>
#include <Platform/Process.hpp>
#include <Core/Result.hpp>

#include <utility>

#ifdef NBL_PLATFORM_LINUX

#include <pthread.h>
#include <atomic>

#endif


namespace nbl {


class Mutex {
public:

    Mutex() = default;
    ~Mutex();

    void lock();
    void unlock();

    static Result create(Mutex* out);


    struct FrameLock {
        ~FrameLock() {
            mutex->unlock();
        }
        Mutex* mutex;
    };

    [[nodiscard]] FrameLock frameLock() {
        lock();
        return { .mutex = this };
    }

private:

#ifdef NBL_PLATFORM_LINUX
    pthread_mutex_t _mHandle;
#else 
    void* _mHandle;
#endif

};


class Condition {
public:

    Condition() = default;
    ~Condition();

    void signal();
    void broadcast();
    void wait();

    static Result create(Condition* out);

private:
#ifdef NBL_PLATFORM_LINUX
    pthread_mutex_t _mMutex;
    pthread_cond_t _mHandle;
#else
    void* _mHandle;
#endif

};


class Latch {
public:

    ~Latch();

    void increment();
    void decrement();
    void wait();
    static Result create(usize count, Latch* out);

private:

#ifdef NBL_PLATFORM_LINUX
    pthread_mutex_t _mMutex;
    pthread_cond_t _mCondition;
    std::atomic_int _mCount;
    //usize _mCount;
#else
    void* _mHandle;
#endif

};


template<typename T>
class Future {
public:

    void complete(const T& value) {
        new (_mValue) T(value);
        _mCondition.broadcast();
    }

    void complete(T&& value) {
        new (_mValue) T(std::move(value));
        _mCondition.broadcast();
    }

    template<typename... Args>
    void emplaceComplete(Args&&... args) {
        new (_mValue) T(std::forward<Args...>(args...));
        _mCondition.broadcast();
    }

    T& wait() {
        _mCondition.wait();
        return *reinterpret_cast<T*>(_mValue);
    }

private:
    Condition _mCondition;
    u8 _mValue[sizeof(T)];
};

}

