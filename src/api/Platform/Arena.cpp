// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Arena.cpp
 * \author jwall64
 * \brief Implementation of Platform/Arena.hpp
 */

#include <Platform/Arena.hpp>

#include "Memory.hpp"

namespace {

thread_local nbl::Arena gThreadArena;

nbl::Result _writeArena(void* impl, const void* data, usize length, usize* bytesWritten) {
    nbl::Arena* arena = static_cast<nbl::Arena*>(impl);
    void* ptr;
    NBL_CHECK(arena->push(length, &ptr));
    memcpy(ptr, data, length);

    if(bytesWritten) {
        *bytesWritten = length;
    }

    return nbl::Result::SUCCESS;
}


}


nbl::Arena* nbl::Arena::threadLocal() {

    if(gThreadArena._pData == nullptr) {
        usize capacity = 1024 * 1024 * 1024; // 1GB
        NBL_ASSERT(nbl::Arena::create(capacity, &gThreadArena).success());
    }

    return &gThreadArena;
}

nbl::Arena::~Arena() {
    if(_pData) {
        usize cap = _mCapacity;
        NBL_ASSERT(nbl::memory::unmapMemory(_pData, &cap).success());
    }
}

nbl::Result nbl::Arena::push(usize size, void** outPtr) {

    if(_mIndex + size > _mCapacity) {
        nbl::log(nbl::LogLevel::DEBUG, "Failed to commit memory! index=", _mIndex, " capacity=", _mCapacity, " requestedSize=", size);
        return nbl::error::OUT_OF_MEMORY;
    }

    if(_mIndex + size > _mCommitted) {
        usize commitSize = (_mIndex + size) - _mCommitted;
        NBL_CHECK(nbl::memory::commitMemory(static_cast<char*>(_pData) + _mCommitted, &commitSize));
        _mCommitted += commitSize;
    }

    void* data = static_cast<char*>(_pData) + _mIndex;

    _mIndex += size;
    memset(data, 0, size);

    if(outPtr) *outPtr = data;

    return Result::SUCCESS;
}

usize nbl::Arena::pop(usize count) {
    if(count > _mIndex) {
        count = _mIndex;
    }

    _mIndex -= count;

    if(_mCommitted - _mIndex >= (nbl::memory::gPageSize * 2)) {
        usize decommitSize = _mCommitted - _mIndex;
        NBL_CHECK(nbl::memory::decommitMemory(static_cast<char*>(_pData) + _mIndex, &decommitSize));
        _mCommitted -= decommitSize;
    }

    return _mIndex;
}

nbl::Arena::operator Writer() {
    return {
        .impl = this,
        .pfnWrite = _writeArena
    };
}

nbl::Result nbl::Arena::create(usize capacity, Arena* outArena) {
    void* data;
    NBL_CHECK(nbl::memory::mapMemory(&data, &capacity));
    outArena->_pData = data;
    outArena->_mCapacity = capacity;
    return Result::SUCCESS;
}
