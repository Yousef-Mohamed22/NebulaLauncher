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
#include <Platform/Logging.hpp>

#include "../Memory.hpp"


#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>



usize nbl::pageSize() noexcept {
    return ::getpagesize();
}

nbl::Result nbl::mapMemory(usize* size, void** outPtr) noexcept {
    if(outPtr == nullptr || size == nullptr || *size == 0) return nbl::Result::INVALID_ARGUMENT;

    *size = nbl::alignUp(*size, getpagesize());
    void* out = ::mmap(nullptr, *size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if(out == nullptr) return nbl::Result::OUT_OF_MEMORY;

    *outPtr = out;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::unmapMemory(void* ptr, usize* size) noexcept {
    if(ptr == nullptr || size == nullptr || *size == 0) return nbl::Result::INVALID_ARGUMENT;

    *size = nbl::alignUp(*size, getpagesize());
    int rc = ::munmap(ptr, *size);

    if(rc != 0) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::commitMemory(void* ptr, usize* size) noexcept {
    NBL_ASSERT(ptr != nullptr);
    NBL_ASSERT(size != nullptr);
    NBL_ASSERT(*size != 0);

    *size = nbl::alignUp(*size, getpagesize());
    int rc = ::mprotect(ptr, *size, PROT_READ | PROT_WRITE);

    if(rc != 0) {
        nbl::log(nbl::LogLevel::DEBUG, "Failed to commit memory! ", errno);
        return nbl::Result::INVALID_ARGUMENT;
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::uncommitMemory(void* ptr, usize* size) noexcept {
    if(ptr == nullptr || size == nullptr || *size == 0) return nbl::Result::INVALID_ARGUMENT;

    *size = nbl::alignUp(*size, getpagesize());
    int rc = ::mprotect(ptr, *size, PROT_NONE);

    if(rc != 0) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    return nbl::Result::SUCCESS;
}


#endif // NBL_PLATFORM_LINUX
