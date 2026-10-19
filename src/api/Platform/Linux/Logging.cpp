/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file Platform/Linux/Logging.cpp
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
