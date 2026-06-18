// Copyright (c) jwall64. All rights reserved.

/**
 * \file DynamicLibrary.cpp
 * \author jwall64
 * \brief Implementation of Platform/DynamicLibrary.hpp for Linux
 */

#include <Platform/PlatformDefines.hpp>
#ifdef NBL_PLATFORM_LINUX

#include <Platform/DynamicLibrary.hpp>

#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

#include <dlfcn.h>


nbl::StringView nbl::dynamicLibraryFileExt() {
    static StringView _ext = ".so";
    return _ext;
}


nbl::DynamicLibrary::~DynamicLibrary() {
    if(_pHandle) {
        ::dlclose(_pHandle);
        _pHandle = nullptr;
    }
}

nbl::Result nbl::DynamicLibrary::_findSymbol(StringView name, void** outSym) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView zName;
    NBL_CHECK(nbl::StringBuilder::nullTerminate(name, &zName, &frame));

    dlerror(); // Clear dlerror
    void* sym = dlsym(_pHandle, name.data);

    const char* err = dlerror();
    if(err) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to find symbol: ", name, " (", err, ")");
        return nbl::error::IO_ERROR;
    }

    *outSym = sym;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::DynamicLibrary::load(StringView path, DynamicLibrary* outLibrary) {

    if(path.length == 0) {
        return nbl::error::ILLEGAL_ARGUMENT;
    }


    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView zPath;
    NBL_CHECK(nbl::StringBuilder::nullTerminate(path, &zPath, &frame));

    void* handle = ::dlopen(zPath.data, RTLD_LAZY);
    if(handle == nullptr) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to load library: ", path, " (", dlerror(), ")");
        return nbl::error::IO_ERROR;
    }

    outLibrary->_pHandle = handle;
    return nbl::Result::SUCCESS;
}


#endif
