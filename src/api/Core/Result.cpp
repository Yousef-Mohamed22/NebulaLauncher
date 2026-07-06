#include <Core/Result.hpp>
#include <Platform/Logging.hpp>

#ifndef NDEBUG 

void nbl::_reportCheckFailed(nbl::Result res, const char* file, i32 line) {
    nbl::log(nbl::LogLevel::WARN, "Check failed! (", res, ") @", file, ":", line); 
}

#endif
