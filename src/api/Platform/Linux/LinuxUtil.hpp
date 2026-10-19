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
 * \file Platform/Linux/LinuxUtil.hpp
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
