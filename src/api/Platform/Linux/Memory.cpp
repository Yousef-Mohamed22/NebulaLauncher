// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Memory.cpp
 * \author jwall64
 * \brief Implementation of Platform/Memory.hpp for Linux.
 */

#include <Platform/PlatformDefines.hpp>

#ifdef NBL_PLATFORM_LINUX

#include <Core/Math.hpp>
#include <Core/Result.hpp>

#include "Platform/Memory.hpp"


#include <unistd.h>
#include <sys/mman.h>


usize nbl::memory::gPageSize = getpagesize();


nbl::Result nbl::memory::mapMemory(void** outPtr, usize* size) {

    *size = nbl::align(*size, gPageSize);

    void* ptr = mmap(nullptr, *size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(ptr == MAP_FAILED) {
        return nbl::error::OUT_OF_MEMORY;
    }
    *outPtr = ptr;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::memory::unmapMemory(void *ptr, usize* size) {
    *size = nbl::align(*size, gPageSize);
    if(munmap(ptr, *size) != 0) {
        return nbl::error::UNEXPECTED_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::memory::commitMemory(void *ptr, usize *size) {
    *size = nbl::align(*size, gPageSize);
    if(mprotect(ptr, *size, PROT_READ | PROT_WRITE) != 0) {
        return nbl::error::OUT_OF_MEMORY;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::memory::decommitMemory(void *ptr, usize *size) {
    *size = nbl::align(*size, gPageSize);
    if(mprotect(ptr, *size, PROT_NONE) != 0) {
        return nbl::error::UNEXPECTED_ERROR;
    }
    return nbl::Result::SUCCESS;
}


#endif // NBL_PLATFORM_LINUX
