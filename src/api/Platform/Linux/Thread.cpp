// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Thread.cpp
 * \author jwall64
 * \brief Partial implementation of Platform/Process.hpp for Linux. Includes thread functions.
 */

#include <Platform/Process.hpp>
#include <Platform/Assert.hpp>
#include <Core/Result.hpp>

#include <pthread.h>


namespace {

struct _ThreadData {
    nbl::Thread::EntryPointFn function;
    void* userData;
};

void* _threadEntryPoint(void* data) {
    _ThreadData* threadData = static_cast<_ThreadData*>(data);
    _ThreadData localData = *threadData;

    delete threadData;

    i32 result = localData.function(localData.userData);
    return reinterpret_cast<void*>(static_cast<intptr_t>(result));
}


}


nbl::Result nbl::Thread::join(i32* outReturnValue) {

    if(!_pHandle) return nbl::Result::INVALID_STATE;

    void* threadResult;
    int result = pthread_join(reinterpret_cast<pthread_t>(_pHandle), &threadResult);
    if(result != 0) {
        return nbl::Result::THREAD_ERROR;
    }

    if(outReturnValue) {
        *outReturnValue = reinterpret_cast<intptr_t>(threadResult);
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Thread::detach() {

    int result = pthread_detach(reinterpret_cast<pthread_t>(_pHandle));
    if(result != 0) {
        return nbl::Result::THREAD_ERROR;
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Thread::create(EntryPointFn function, void* userData, Thread* outThread) {

    NBL_ASSERT(outThread != nullptr);

    _ThreadData* threadData = new _ThreadData{ function, userData };

    pthread_t handle;
    int result = pthread_create(&handle, nullptr, _threadEntryPoint, threadData);
    if(result != 0) {
        delete threadData;
        return nbl::Result::THREAD_ERROR;
    }

    outThread->_pHandle = reinterpret_cast<void*>(handle);
    outThread->_mRunning = true;

    return nbl::Result::SUCCESS;

}
