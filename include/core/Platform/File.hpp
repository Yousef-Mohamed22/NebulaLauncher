// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file File.hpp
 * \author jwall64
 * \brief A simple implementation of a file channel for reading and writing files.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>

namespace nbl {

enum class FileOpenFlag : u8 {
    READ   = 0b0001,
    WRITE  = 0b0010,
    CREATE = 0b0100,
    APPEND = 0b1000
};

typedef u8 FileOpenFlags;

inline FileOpenFlags operator&(FileOpenFlag flag, FileOpenFlag other) {
    return static_cast<u8>(flag) & static_cast<u8>(other);
}

inline FileOpenFlags operator&(FileOpenFlags existing, FileOpenFlag flag) {
    return static_cast<u8>(flag) & existing;
}

inline FileOpenFlags operator&(FileOpenFlag flag, FileOpenFlags existing) {
    return static_cast<u8>(flag) & existing;
}

inline FileOpenFlags operator|(FileOpenFlag flag, FileOpenFlag other) {
    return static_cast<u8>(flag) | static_cast<u8>(other);
}

inline FileOpenFlags operator|(FileOpenFlag flag, FileOpenFlags existing) {
    return static_cast<u8>(flag) | existing;
}

inline FileOpenFlags operator|(FileOpenFlags existing, FileOpenFlag flag) {
    return static_cast<u8>(flag) | existing;
}


enum class FileSeekOrigin : u8 {
    BEGIN,
    CURRENT,
    END
};


class FileChannel {
public:
    FileChannel() : _mHandle(nullptr) {}

    ~FileChannel() { _close(); }

    Writer writer();

    Reader reader();

    nbl::Result seek(isize offset, FileSeekOrigin origin, usize* outOffset);

    nbl::Result offset(usize* outOffset);

    static Result open(StringView path, FileChannel* outChannel, FileOpenFlag flag) {
        return open(path, outChannel, static_cast<FileOpenFlags>(flag));
    }

    static Result open(StringView path, FileChannel* outChannel, FileOpenFlags flags);

private:

    void _close();

    void* _mHandle;
};


StringView getWorkingDirectory();

StringView getDataDirectory();


}
