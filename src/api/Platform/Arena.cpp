// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Arena.cpp
 * \author jwall64
 * \brief Implementation of Platform/Arena.hpp
 */

#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

#include "Memory.hpp"
#include "Platform/Allocator.hpp"


namespace {

thread_local nbl::Arena _sThreadArena;

}

nbl::Result nbl::Arena::push(usize amount, void** outPtr) noexcept {
    
    if(amount == 0) return nbl::Result::INVALID_ARGUMENT;

    void* out = end();
    usize remaining = _mCommitted - _mIndex;
    if(remaining < amount) {
        if(_mCommitted >= _mCapacity) {
            NBL_ERROR("Out of memory! ", amount, " > ", remaining);
            return nbl::Result::OUT_OF_MEMORY;
        }

        u8* allocatedEnd = static_cast<u8*>(_pHead) + _mCommitted;
        usize needed = amount - remaining;
        usize committed = needed;
        NBL_CHECK(nbl::commitMemory(allocatedEnd, &committed));
        _mCommitted += committed;
    }

    _mIndex += amount;
    if(outPtr) *outPtr = out;
    memset(out, 0, amount);

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Arena::pop(usize amount) noexcept {

    if(amount == 0) return nbl::Result::SUCCESS;
    if(amount > _mIndex) return nbl::Result::INVALID_ARGUMENT;

    _mIndex -= amount;
    usize remaining = _mCommitted - _mIndex;
    usize pageSize = nbl::pageSize();

    usize remainingPages = remaining / pageSize;
    if(remainingPages > 1) {
        usize uncommitted = (remainingPages - 1) * pageSize;
        u8* allocatedEnd = static_cast<u8*>(_pHead) + _mCommitted;
        NBL_CHECK(nbl::uncommitMemory(allocatedEnd - uncommitted, &uncommitted));

        _mCommitted -= uncommitted;
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::Arena::create(usize amount, Arena* outArena) noexcept {

    usize mapped = amount;
    void* data = nullptr;
    NBL_CHECK(nbl::mapMemory(&mapped, &data));

    outArena->_pHead = data;
    outArena->_mCapacity = mapped;
    outArena->_mCommitted = 0;
    outArena->_mIndex = 0;

    return nbl::Result::SUCCESS;
}

nbl::Arena* nbl::Arena::threadLocal() noexcept {

    if(_sThreadArena._mCapacity == 0) {
        NBL_ASSERT(nbl::Arena::create(1024 * 1024 * 32, &_sThreadArena) == nbl::Result::SUCCESS);
    }

    return &_sThreadArena;
}

bool nbl::Arena::isThreadLocalWriter(Writer writer) noexcept {

    const nbl::LinearAllocator* alloc = nullptr;
    if(nbl::StringBuilder::isStringBuilderWriter(writer)) {
        alloc = static_cast<nbl::StringBuilder*>(writer.impl)->allocator();
    }
    else if(nbl::LinearAllocator::isLinearWriter(writer)) {
        alloc = static_cast<nbl::LinearAllocator*>(writer.impl);
    }
    else {
        return false;
    }

    if(const Arena* arena = dynamic_cast<const nbl::Arena*>(alloc)) {
        return arena == threadLocal();
    }
    if(const ArenaFrame* frame = dynamic_cast<const nbl::ArenaFrame*>(alloc)) {
        return frame->_pArena == threadLocal();
    }

    return false;
}
