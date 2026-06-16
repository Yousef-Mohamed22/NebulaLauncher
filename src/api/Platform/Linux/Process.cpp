// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Process.cpp
 * \author jwall64
 * \brief Partial implementation of Platform/Process.hpp for Linux. Includes process creation and management functions.
 */

#include <Platform/PlatformDefines.hpp>
#ifdef NBL_PLATFORM_LINUX

#include <Core/Result.hpp>
#include <Core/Array.hpp>
#include <Core/String.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Process.hpp>
#include <Platform/StringBuilder.hpp>

#include "Platform/Linux/LinuxUtil.hpp"

#include <cstdlib>
#include <linux/limits.h>
#include <sys/wait.h>
#include <fcntl.h>


namespace {


int _getHandle(void* handle) {
    return static_cast<int>(reinterpret_cast<intptr_t>(handle));
}

void _setHandle(void* handle, int value) {
    handle = reinterpret_cast<void*>(static_cast<intptr_t>(value));
}

}


nbl::StringView nbl::getEnv(nbl::StringView name) {
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView data;
    if(nbl::StringBuilder::nullTerminate(name, &data, &frame).error()) return "";
    return ::getenv(data.data);
}


nbl::StringView nbl::getBinaryPath() {

    static char path[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", path, PATH_MAX - 1);
    NBL_ASSERT(len != -1);

    path[len] = '\0';
    return nbl::StringView(len, path);
}




nbl::Result nbl::ProcessHandle::wait(i32* exitCode) const {
    pid_t rc = waitpid(_getHandle(_pHandle), exitCode ? reinterpret_cast<int*>(exitCode) : nullptr, 0);
    if(rc == -1) {
        return nbl::error::PROCESS_ERROR;
    }
    return nbl::Result::SUCCESS;
}

nbl::Reader nbl::ProcessHandle::stdoutReader() const {
    if(!captureOutput()) {
        return nbl::NullStream();
    }
    return nbl::Reader{
        .impl = _pOutputHandle,
        .pfnRead = nbl::linux::readFd
    };
}

nbl::Result nbl::ProcessHandle::run(StringView commandLine, ProcessHandle* outHandle, ProcessCreateFlags flags) {
    nbl::Array<StringView, 2> args = {
        "-c",
        commandLine
    };
    return spawn("/bin/sh", args, outHandle, flags);
}

nbl::Result nbl::ProcessHandle::spawn(StringView path, Span<StringView> args, ProcessHandle* outHandle, ProcessCreateFlags flags) {

    int pipefd[2];
    if(flags & ProcessCreateFlag::CAPTURE_OUTPUT) {
        if(pipe(pipefd) == -1) {
            return nbl::error::PROCESS_ERROR;
        }
    }

    pid_t pid = fork();
    if(pid == -1) {
        return nbl::error::PROCESS_ERROR;
    }

    if(pid == 0) {

        // Handle standard IO
        if(flags & ProcessCreateFlag::CAPTURE_OUTPUT) {
            int outputHandle = _getHandle(outHandle->_pOutputHandle);
            close(pipefd[0]);
            if(dup2(outputHandle, STDOUT_FILENO) == -1 || dup2(outputHandle, STDERR_FILENO) == -1) {
                close(outputHandle);
                exit(1);
            }
            int nullfd = open("/dev/null", O_RDWR);
            if(nullfd == -1) {
                close(outputHandle);
                exit(1);
            }
            if(dup2(nullfd, STDIN_FILENO) == -1) {
                close(outputHandle);
                close(nullfd);
                exit(1);
            }

        } else {

            int fd = open("/dev/null", O_RDWR);
            if(fd == -1) {
                exit(1);
            }

            if(dup2(fd, STDIN_FILENO) == -1 || dup2(fd, STDOUT_FILENO) == -1 || dup2(fd, STDERR_FILENO) == -1) {
                close(fd);
                exit(1);
            }
        }

        // Start the process
        nbl::ArenaFrame frame = nbl::ArenaFrame(nbl::Arena::threadLocal());

        char** argv;
        NBL_CHECK(frame.allocate(&argv, sizeof(const char**) * (args.length + 2)));

        NBL_CHECK(frame.allocate(&argv[0], path.length + 1));
        memcpy(argv[0], path.data, path.length);
        argv[0][path.length] = '\0';

        for(usize i = 0; i < args.length; i++) {
            NBL_CHECK(frame.allocate(&argv[i + 1], args[i].length + 1));

            memcpy(argv[i + 1], args[i].data, args[i].length);
            argv[i + 1][args[i].length] = '\0';
        }
        argv[args.length + 1] = nullptr;

        if(execvp(argv[0], argv) != 0) {
            return nbl::error::PROCESS_ERROR;
        }

    } else {

        // Parent process
        close(pipefd[1]);
        _setHandle(outHandle->_pOutputHandle, pipefd[0]);

        outHandle->_pHandle = reinterpret_cast<void*>(static_cast<intptr_t>(pid));
    }

    return Result::SUCCESS;
}

#endif
