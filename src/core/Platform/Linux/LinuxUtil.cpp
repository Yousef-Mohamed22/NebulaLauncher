// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file LinuxUtil.cpp
 * \author jwall64
 * \brief Implementation of Linux-specific utility functions.
 */

#include "LinuxUtil.hpp"
#include <Core/Result.hpp>

#include <errno.h>
#include <unistd.h>

nbl::Result nbl::linux::writeFd(void* impl, const void* data, usize length, usize* bytesWritten) {

    int fd = static_cast<int>(reinterpret_cast<intptr_t>(impl));

    ssize_t result = ::write(fd, data, length);
    if(result < 0) {
        return nbl::error::IO_ERROR;
    }
    if(bytesWritten) {
        *bytesWritten = static_cast<usize>(result);
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::linux::readFd(void* impl, void* data, usize length, usize* bytesRead) {

    int fd = static_cast<int>(reinterpret_cast<intptr_t>(impl));

    ssize_t result = ::read(fd, data, length);
    if(result < 0) {
        if(errno == EINTR) {
            return nbl::Result::SUCCESS;
        }
        return nbl::error::IO_ERROR;
    }
    if(result == 0) {
        return nbl::result::END_OF_FILE;
    }
    if(bytesRead) {
        *bytesRead = static_cast<usize>(result);
    }
    return nbl::Result::SUCCESS;
}
