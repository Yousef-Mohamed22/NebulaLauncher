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
#include <time.h>
#include <limits.h>
#include <errno.h>

#include <linux/limits.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

namespace {


int _getHandle(void* handle) {
    return static_cast<int>(reinterpret_cast<intptr_t>(handle));
}

void _setHandle(void** handle, int value) {
    *handle = reinterpret_cast<void*>(static_cast<intptr_t>(value));
}

}



nbl::StringView nbl::getHostName() {

    static char hostname[HOST_NAME_MAX];
    static StringView out = {};
    if(out.length == 0) {
        gethostname(hostname, HOST_NAME_MAX-1);
        out.length = strlen(hostname);
        out.data = hostname;
    }

    return out;
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
    return nbl::StringView(path, len);
}


nbl::Result nbl::sleep(u32 millis) {
    timespec ts{};
    ts.tv_nsec = (millis * 1000000) % 1000000000;
    ts.tv_sec = (millis / 1000);

    while(true) {
        int rc = nanosleep(&ts, &ts);
        if(rc == -1) {
            if(errno == EINTR) {
                continue;
            }
            if(errno == EINVAL) {
                return nbl::Result::INVALID_ARGUMENT;
            }
        } else {
            return nbl::Result::SUCCESS;
        }
    }
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
        return nbl::Result::PROCESS_ERROR;
    }
    return nbl::Result::SUCCESS;
}

bool nbl::ProcessHandle::isRunning() const {
    if(_pHandle == nullptr) return false;

    int pid = _getHandle(_pHandle);
    return waitpid(pid, nullptr, WNOHANG) == 0;
}

void nbl::ProcessHandle::terminate() {
    if(_pHandle == nullptr) return;

    int pid = _getHandle(_pHandle);
    ::kill(pid, SIGTERM);
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

nbl::Result nbl::ProcessHandle::run(StringView commandLine, ProcessHandle* outHandle, const ProcessCreateOptions& options) {
    nbl::Array<StringView, 2> args = {
        "-c",
        commandLine
    };
    return spawn("/bin/sh", args, outHandle, options);
}

nbl::Result nbl::ProcessHandle::spawn(StringView path, Span<StringView> args, ProcessHandle* outHandle, const ProcessCreateOptions& options) {

    int pipefd[2];
    if(options.outputMode == ProcessOutputMode::PIPE) {
        if(pipe(pipefd) == -1) {
            return nbl::Result::PROCESS_ERROR;
        }
    } else if(options.outputMode == ProcessOutputMode::REDIRECT) {
        if(options.outputRedirectFile.empty() || options.outputRedirectFile.length + 1 >= PATH_MAX) {
            return nbl::Result::INVALID_ARGUMENT;
        }
    }

    pid_t pid = fork();
    if(pid == -1) {
        return nbl::Result::PROCESS_ERROR;
    }

    if(pid == 0) {

        if(options.killOnParentExit) {
            if(prctl(PR_SET_PDEATHSIG, SIGKILL) == -1) {
                exit(1);
            }
        }

        if(!options.workingDir.empty()) {

            nbl::ArenaFrame frame(nbl::Arena::threadLocal());
            nbl::StringView path;
            if(nbl::StringBuilder::nullTerminate(options.workingDir, &path, &frame).error()) {
                exit(1);
            }

            chdir(path.data);
        }

        // Handle standard IO
        int nullfd = open("/dev/null", O_RDWR);
        if(nullfd == -1) {
            exit(1);
        }
        if(dup2(nullfd, STDIN_FILENO) == -1) {
            close(nullfd);
            exit(1);
        }

        switch(options.outputMode) {
            case ProcessOutputMode::PIPE: {
                if(dup2(pipefd[1], STDOUT_FILENO) == -1 || dup2(pipefd[1], STDERR_FILENO) == -1) {
                    close(nullfd);
                    close(pipefd[0]);
                    close(pipefd[1]);
                    exit(1);
                }
                close(pipefd[0]);
                close(pipefd[1]);
                break;
            }
            case ProcessOutputMode::NULLIFY: {
                if(dup2(nullfd, STDOUT_FILENO) == -1 || dup2(nullfd, STDERR_FILENO) == -1) {
                    close(nullfd);
                    exit(1);
                }
                break;
            }
            case ProcessOutputMode::REDIRECT: {
                char nullPath[PATH_MAX];
                memcpy(nullPath, options.outputRedirectFile.data, options.outputRedirectFile.length);
                nullPath[options.outputRedirectFile.length] = '\0';

                int fd = ::open(nullPath, O_WRONLY | O_CREAT);
                if(fd == -1) {
                    close(nullfd);
                    exit(1);
                }

                if(dup2(fd, STDOUT_FILENO) == -1 || dup2(fd, STDERR_FILENO) == -1) {
                    close(fd);
                    close(nullfd);
                    exit(1);
                }
                break;
            }
            case ProcessOutputMode::INHERIT:
            default:
                break;
        }

        close(nullfd);

        // Start the process
        nbl::ArenaFrame frame = nbl::ArenaFrame(nbl::Arena::threadLocal());

        char** argv;
        if(frame.allocate(&argv, sizeof(const char**) * (args.length + 2)).error()) {
            exit(1);
        }

        if(frame.allocate(&argv[0], path.length + 1).error()) {
            exit(1);
        }
        memcpy(argv[0], path.data, path.length);
        argv[0][path.length] = '\0';

        for(usize i = 0; i < args.length; i++) {
            if(frame.allocate(&argv[i + 1], args[i].length + 1).error()) {
                exit(1);
            }

            memcpy(argv[i + 1], args[i].data, args[i].length);
            argv[i + 1][args[i].length] = '\0';
        }
        argv[args.length + 1] = nullptr;

        if(execvp(argv[0], argv) != 0) {
            exit(1);
        }

    } else {

        // Parent process
        if(options.outputMode == ProcessOutputMode::PIPE) {
            _setHandle(&outHandle->_pOutputHandle, pipefd[0]);
            ::close(pipefd[1]);
        }

        _setHandle(&outHandle->_pHandle, pid);
    }

    return Result::SUCCESS;
}

nbl::Result nbl::ProcessHandle::fromPid(i32 pid, nbl::ProcessHandle* out) {
    out->_pHandle = reinterpret_cast<void*>(static_cast<intptr_t>(pid));
    return nbl::Result::SUCCESS;
}

#endif
