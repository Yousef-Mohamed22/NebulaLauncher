// Copyright (c) jwall64. All rights reserved.

/**
 * \file DynamicLibrary.hpp
 * \author jwall64
 * \brief Interface for loading dynamic libraries
 */

#pragma once

#include "Core/Result.hpp"
#include <Core/String.hpp>

namespace nbl {

/**
 * \brief Gets the platform-specific file extension for a dynamic library file.
 * \return The dynamic library file extension, including the leading period. (i.e. ".so")
 */
StringView dynamicLibraryFileExt();


/**
 * \brief A loaded dynamic library
 */
class DynamicLibrary {
public:

    DynamicLibrary() = default;

    DynamicLibrary(const DynamicLibrary& other) = delete;
    inline DynamicLibrary(DynamicLibrary&& other) noexcept :
        _pHandle(other._pHandle) 
    {
        other._pHandle = nullptr;
    }

    ~DynamicLibrary();

    DynamicLibrary& operator=(const DynamicLibrary& other) = delete;
    DynamicLibrary& operator=(DynamicLibrary&& other) noexcept {
        if(&other == this) return *this;
        _pHandle = other._pHandle;
        other._pHandle = nullptr;
        return *this;
    }

    /**
     * \brief Attempts to get the address of a symbol exported by the library.
     * \name The name of the symbol to lookup.
     * \outSym a pointer to the pointer which will receive the symbol's address.
     * \return A result representing the success or failure of the lookup operation.
     */
    template<typename T>
    Result findSymbol(StringView name, T* outSym) {
        return _findSymbol(name, reinterpret_cast<void**>(outSym));
    }

    /**
     * \brief Loads a dynamic library from disk.
     * \param Path The path to the dynamic library file.
     * \param outLibrary The library handle to initialize.
     * \return A result representing the success or failure of the load operation.
     */
    static Result load(StringView path, DynamicLibrary* outLibrary);

private:

    Result _findSymbol(StringView name, void** outSym);

    void* _pHandle;

};


}
