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

namespace {

struct _ThreadData {
    pthread_t handle;
    nbl::Thread::EntryPointFn function;
    void* userData;
    u32 refCount;
};

void* _threadEntryPoint(void* data) {
    _ThreadData* threadData = static_cast<_ThreadData*>(data);
    threadData->refCount += 1;

    i32 result = threadData->function(threadData->userData);
    
    threadData->refCount -= 1;
    if(threadData->refCount == 0) {
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
    void* threadResult;
    int result = pthread_join(data->handle, &threadResult);
    if(result != 0) {
        return nbl::Result::THREAD_ERROR;
    }

    if(outReturnValue) {
        *outReturnValue = reinterpret_cast<intptr_t>(threadResult);
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Thread::detach() {

    if(_pHandle) {

        _ThreadData* data = static_cast<_ThreadData*>(_pHandle);
        _pHandle = nullptr;

        data->refCount -= 1;

        if(data->refCount > 0) { // Thread is alive
            int result = pthread_detach(data->handle);
            if(result != 0) {
                return nbl::Result::THREAD_ERROR;
            }
        } else {
            delete data;
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
        .refCount = 1 
    };

    int result = pthread_create(&threadData->handle, nullptr, _threadEntryPoint, threadData);
    if(result != 0) {
        delete threadData;
        return nbl::Result::THREAD_ERROR;
    }

    outThread->_pHandle = threadData;

    return nbl::Result::SUCCESS;

}
