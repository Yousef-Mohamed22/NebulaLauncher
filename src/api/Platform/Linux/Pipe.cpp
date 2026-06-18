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
        return nbl::error::IO_ERROR;
    }

    outPipe->_mReadEndHandle = reinterpret_cast<void*>(static_cast<intptr_t>(fds[0]));
    outPipe->_mWriteEndHandle = reinterpret_cast<void*>(static_cast<intptr_t>(fds[1]));

    return nbl::Result::SUCCESS;
}


#endif
