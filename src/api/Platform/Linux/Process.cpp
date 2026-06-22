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

#include "LinuxUtil.hpp"

#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#include <linux/limits.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

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


nbl::ProcessHandle::~ProcessHandle() {
    if(_pHandle != nullptr) {
        pid_t pid = _getHandle(_pHandle);
        close(pid);
        _pHandle = nullptr;
    }
    if(captureOutput()) {
        int fd = _getHandle(_pOutputHandle);
        close(fd);
        _pOutputHandle = nullptr;
    }
}


nbl::Result nbl::ProcessHandle::wait(i32* exitCode) const {

    int pid = _getHandle(_pHandle);
    if(waitpid(pid, exitCode, 0) == -1) {
        return nbl::error::PROCESS_ERROR;
    }
    return nbl::Result::SUCCESS;
}

bool nbl::ProcessHandle::isRunning() const {
    if(_pHandle == nullptr) return false;

    int pid = _getHandle(_pHandle);
    return waitpid(pid, nullptr, WNOHANG) == 0;
}

void nbl::ProcessHandle::kill() {
    if(_pHandle == nullptr) return;

    int pid = _getHandle(_pHandle);
    ::kill(pid, SIGKILL);
}

i32 nbl::ProcessHandle::getPid() const {
    return _getHandle(_pHandle);
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

        if(flags & ProcessCreateFlag::KILL_ON_PARENT_EXIT) {
            prctl(PR_SET_PDEATHSIG, SIGKILL);
        }
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
        if(flags & ProcessCreateFlag::CAPTURE_OUTPUT) {
            close(pipefd[1]);
            _setHandle(outHandle->_pOutputHandle, pipefd[0]);
        }

        outHandle->_pHandle = reinterpret_cast<void*>(static_cast<intptr_t>(pid));
    }

    return Result::SUCCESS;
}

nbl::Result nbl::ProcessHandle::fromPid(i32 pid, nbl::ProcessHandle* out) {
    out->_pHandle = reinterpret_cast<void*>(static_cast<intptr_t>(pid));
    return nbl::Result::SUCCESS;
}

#endif
