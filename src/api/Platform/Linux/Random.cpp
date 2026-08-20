#include <Platform/Random.hpp>

#include <Platform/PlatformDefines.hpp>

#ifdef NBL_PLATFORM_LINUX

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

nbl::Result nbl::secureRandom(Span<u8> buffer) {

    if(buffer.length == 0) return nbl::Result::INVALID_ARGUMENT;

    int fd = ::open("/dev/urandom", O_RDONLY);
    if(fd == -1) return nbl::Result::IO_ERROR;

    size_t totalRead = 0;
    ssize_t justRead = 0;

    while((justRead = ::read(fd, buffer.data + totalRead, buffer.length - totalRead)) > 0) {
        totalRead += justRead;
    }
    if(justRead == -1) return nbl::Result::IO_ERROR;

    return nbl::Result::SUCCESS;
}

#endif
