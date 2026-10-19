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
 * \file Platform/Linux/Pipe.cpp
 * \author jwall64
 * \brief Implementation of Platform/Pipe.hpp for Linux.
 */

#include <Platform/PlatformDefines.hpp>
#ifdef NBL_PLATFORM_LINUX

#include <Core/Result.hpp>
#include <Platform/Pipe.hpp>
  
#include "LinuxUtil.hpp"

#include <unistd.h>

void nbl::Pipe::close() {
    if(_mReadEndHandle) {
        ::close(static_cast<int>(reinterpret_cast<intptr_t>(_mReadEndHandle)));
    }
    if(_mWriteEndHandle) {
        ::close(static_cast<int>(reinterpret_cast<intptr_t>(_mWriteEndHandle)));
    }
}

void nbl::Pipe::closeWriter() {
    if(_mWriteEndHandle) {
        ::close(static_cast<int>(reinterpret_cast<intptr_t>(_mWriteEndHandle)));
    }
}

nbl::Reader nbl::Pipe::reader() {
    return Reader {
        .impl = _mReadEndHandle,
        .pfnRead = linux::readFd
    };
}

nbl::Writer nbl::Pipe::writer() {
    return Writer {
        .impl = _mWriteEndHandle,
        .pfnWrite = linux::writeFd
    };
}

nbl::Result nbl::Pipe::create(nbl::Pipe* outPipe) {

    int fds[2];
    int rc = ::pipe(fds);

    if(rc < 0) {
        return nbl::Result::IO_ERROR;
    }

    outPipe->_mReadEndHandle = reinterpret_cast<void*>(static_cast<intptr_t>(fds[0]));
    outPipe->_mWriteEndHandle = reinterpret_cast<void*>(static_cast<intptr_t>(fds[1]));

    return nbl::Result::SUCCESS;
}


#endif
