// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Thread.cpp
 * \author jwall64
 * \brief Partial implementation of Platform/Process.hpp for Linux. Includes thread functions.
 */

#include <Platform/Process.hpp>
#include <Platform/Assert.hpp>
#include <Core/Result.hpp>
#include <Platform/Logging.hpp>

#include <pthread.h>
#include <atomic>

namespace {

struct _ThreadData {
    pthread_t handle;
    nbl::Thread::EntryPointFn function;
    void* userData;
    std::atomic_int refCount;
};

void* _threadEntryPoint(void* data) {
    _ThreadData* threadData = static_cast<_ThreadData*>(data);

    i32 result = threadData->function(threadData->userData);
    
    int old = threadData->refCount.fetch_sub(1);
    if(old == 1) {
        delete threadData;
    } 

    return reinterpret_cast<void*>(static_cast<intptr_t>(result));
}


}

nbl::Thread::~Thread() {
    detach();
}


bool nbl::Thread::isRunning() const {
    if(_pHandle == nullptr) return false;

    _ThreadData* data = static_cast<_ThreadData*>(_pHandle);
    return data->refCount == 2;
}


nbl::Result nbl::Thread::join(i32* outReturnValue) {

    if(!_pHandle) return nbl::Result::SUCCESS;

    _ThreadData* data = static_cast<_ThreadData*>(_pHandle);
    _pHandle = nullptr;

    int old = data->refCount.fetch_sub(1);
    if(old == 1) {
        if(outReturnValue) *outReturnValue = 0;
        delete data;
    } else {
        void* ret;
        int result = pthread_join(data->handle, &ret);
        if(result != 0) {
            return nbl::Result::THREAD_ERROR;
        }
        if(outReturnValue) {
            *outReturnValue = reinterpret_cast<intptr_t>(ret);
        }
    }


    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Thread::detach() {

    if(_pHandle) {

        _ThreadData* data = static_cast<_ThreadData*>(_pHandle);
        _pHandle = nullptr;

        int old = data->refCount.fetch_sub(1);
        if(old == 1) {
            delete data;
        } else {
            int result = pthread_detach(data->handle);
            if(result != 0) {
                return nbl::Result::THREAD_ERROR;
            }
        }
    }

    return nbl::Result::SUCCESS;

}

nbl::Result nbl::Thread::create(EntryPointFn function, void* userData, Thread* outThread) {

    NBL_ASSERT(outThread != nullptr);

    _ThreadData* threadData = new _ThreadData { 
        .handle = {}, 
        .function = function, 
        .userData = userData, 
        .refCount = 2
    };

    int result = pthread_create(&threadData->handle, nullptr, _threadEntryPoint, threadData);
    if(result != 0) {
        nbl::log(nbl::LogLevel::DEBUG, "Thread create failed: ", result);
        delete threadData;
        return nbl::Result::THREAD_ERROR;
    }

    outThread->_pHandle = threadData;
    return nbl::Result::SUCCESS;

}
