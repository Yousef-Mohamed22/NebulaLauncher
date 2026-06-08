// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file LinuxUtil.hpp
 * \author jwall64
 * \brief Declarations for Linux-specific utility functions.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Stream.hpp>

#include <unistd.h>

namespace nbl::linux {

nbl::Result writeFd(void* impl, const void* data, usize length, usize* bytesWritten);
nbl::Result readFd(void* impl, void* data, usize length, usize* bytesRead);

inline nbl::Writer fdWriter(int fd) {
    return nbl::Writer {
        .impl = reinterpret_cast<void*>(static_cast<intptr_t>(fd)),
        .pfnWrite = nbl::linux::writeFd
    };
}

}
