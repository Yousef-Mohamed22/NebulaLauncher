// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Pipe.hpp
 * \author jwall64
 * \brief Contains a definition of a pipe interface
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>

namespace nbl {

class Pipe {
public:

    Pipe() = default;
    ~Pipe() { close(); }

    operator Reader() { return reader(); }
    operator Writer() { return writer(); }

    Reader reader();
    Writer writer();

    void close();
    void closeWriter();

    static Result create(Pipe* outPipe);

private:

    void* _mReadEndHandle;
    void* _mWriteEndHandle;

};

}
