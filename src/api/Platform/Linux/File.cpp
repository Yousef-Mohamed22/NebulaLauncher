// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file File.cpp
 * \author jwall64
 * \brief Implementation of Platform/File.hpp for Linux.
 */

#include <Platform/Assert.hpp>
#include <Platform/File.hpp>
#include <Platform/Socket.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

#include "LinuxUtil.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace {

char gCwd[PATH_MAX];
char gDataDir[PATH_MAX];


int gGetFd(void* handle) {
    return static_cast<int>(reinterpret_cast<intptr_t>(handle));
}

}


nbl::Writer nbl::FileChannel::writer() {
    return {
        .impl = _mHandle,
        .pfnWrite = nbl::linux::writeFd
    };
}

nbl::Reader nbl::FileChannel::reader() {
    return {
        .impl = _mHandle,
        .pfnRead = nbl::linux::readFd
    };
}

nbl::Result nbl::FileChannel::seek(isize offset, FileSeekOrigin origin, usize* outOffset) {
    int whence = SEEK_SET;
    switch(origin) {
        case FileSeekOrigin::BEGIN:
            whence = SEEK_SET;
            break;
        case FileSeekOrigin::CURRENT:
            whence = SEEK_CUR;
            break;
        case FileSeekOrigin::END:
            whence = SEEK_END;
            break;
    }

    off_t result = ::lseek(gGetFd(_mHandle), static_cast<off_t>(offset), whence);
    if(result < 0) {
        return nbl::error::IO_ERROR;
    }
    if(outOffset) {
        *outOffset = static_cast<usize>(result);
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::FileChannel::offset(usize* outOffset) {
    off_t current = ::lseek(gGetFd(_mHandle), 0, SEEK_CUR);
    if(current < 0) {
        return nbl::error::IO_ERROR;
    }
    if(outOffset) {
        *outOffset = static_cast<usize>(current);
    }
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::FileChannel::open(StringView path, FileChannel* outChannel, FileOpenFlags flags) {
    int openFlags = 0;

    if(flags & FileOpenFlag::READ && flags & FileOpenFlag::WRITE) {
        openFlags |= O_RDWR;
    } else if(flags & FileOpenFlag::READ) {
        openFlags |= O_RDONLY;
    } else if(flags & FileOpenFlag::WRITE) {
        openFlags |= O_WRONLY;    
    } else {
        return nbl::error::ILLEGAL_ARGUMENT;
    }

    if(flags & FileOpenFlag::CREATE) {
        openFlags |= O_CREAT;
    }
    if(flags & FileOpenFlag::APPEND) {
        openFlags |= O_APPEND;
    }

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    StringView nullTerminatedPath;
    NBL_CHECK(nbl::StringBuilder::nullTerminate(path, &nullTerminatedPath, &frame));

    int fd = ::open(nullTerminatedPath.data, openFlags, 0644);
    if(fd < 0) {
        return nbl::error::IO_ERROR;
    }

    outChannel->_mHandle = reinterpret_cast<void*>(static_cast<intptr_t>(fd));
    return nbl::Result::SUCCESS;
}

void nbl::FileChannel::_close() {
    if(_mHandle) {
        ::close(gGetFd(_mHandle));
        _mHandle = nullptr;
    }
}

nbl::StringView nbl::getWorkingDirectory() {
    if(gCwd[0] == '\0') {
        NBL_ASSERT(getcwd(gCwd, PATH_MAX) != nullptr);
    }
    return StringView(gCwd);
}

nbl::StringView nbl::getDataDirectory() {
    if(gDataDir[0] == '\0') {

        const char* xdgDataHome = getenv("XDG_DATA_HOME");
        if(xdgDataHome && xdgDataHome[0] != '\0') {
            strlcpy(gDataDir, xdgDataHome, PATH_MAX);
        } else {
            const char* home = getenv("HOME");
            if(home && home[0] != '\0') {
                strlcpy(gDataDir, home, PATH_MAX);
                strlcat(gDataDir, "/.local/share", PATH_MAX);
            } else {
                // Fallback to current working directory if HOME is not set
                NBL_ASSERT(getcwd(gDataDir, PATH_MAX) != nullptr);
            }
        }

    }
    return StringView(gDataDir);
}


nbl::Writer nbl::Socket::writer() {
    return {
        .impl = _mHandle,
        .pfnWrite = nbl::linux::writeFd
    };
}

nbl::Reader nbl::Socket::reader() {
    return {
        .impl = _mHandle,
        .pfnRead = nbl::linux::readFd
    };
}

void nbl::Socket::close() {
    if(_mHandle) {
        ::close(gGetFd(_mHandle));
        _mHandle = nullptr;
    }
}

nbl::Result nbl::Socket::connectIPv4(u32 address, u16 port, SocketType type, Socket* outSocket) {

    int sockType;
    if(type == SocketType::TCP) {
        sockType = SOCK_STREAM;
    } else if(type == SocketType::UDP) {
        sockType = SOCK_DGRAM;
    } else {
        return nbl::error::ILLEGAL_ARGUMENT;
    }

    int fd = ::socket(AF_INET, sockType, 0);

    if(fd < 0) {
        return nbl::error::IO_ERROR;
    }

    sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = port,
        .sin_addr = {
            .s_addr = address
        }
    };

    if(::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(sockaddr_in)) < 0) {
        ::close(fd);
        return nbl::error::IO_ERROR;
    }

    outSocket->_mHandle = reinterpret_cast<void*>(static_cast<intptr_t>(fd));
    return nbl::Result::SUCCESS;
}
