// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Logging.cpp
 * \author jwall64
 * \brief Implementation of Platform/Logging.hpp for Linux.
 */

#include <Platform/PlatformDefines.hpp>

#ifdef NBL_PLATFORM_LINUX

#include <Platform/Logging.hpp>

#include <unistd.h>

#include "LinuxUtil.hpp"


void nbl::stdout(StringView data) {
    ::write(STDOUT_FILENO, data.data, data.length);
}

void nbl::stderr(StringView data) {
    ::write(STDERR_FILENO, data.data, data.length);
}

nbl::Writer nbl::stdoutWriter() {
    return nbl::linux::fdWriter(STDOUT_FILENO);
}

nbl::Writer nbl::stderrWriter() {
    return nbl::linux::fdWriter(STDERR_FILENO);
}


#endif // NBL_PLATFORM_LINUX
