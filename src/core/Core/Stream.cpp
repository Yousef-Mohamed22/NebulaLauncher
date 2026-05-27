// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Stream.cpp
 * \author jwall64
 * \brief Implementation for Core/Stream.hpp
 */

#include <Core/Stream.hpp>

#include <cstring>

nbl::Result nbl::BufferStream::_write(void* impl, const void* data, usize length, usize* bytesWritten) {

    nbl::BufferStream* writer = static_cast<nbl::BufferStream*>(impl);

    if(writer->_mWriterIndex + length > writer->_mBuffer.length) {
        return nbl::error::OUT_OF_MEMORY;
    }
    memcpy(writer->_mBuffer.data + writer->_mWriterIndex, data, length);
    writer->_mWriterIndex += length;
    if(bytesWritten) {
        *bytesWritten = length;
    }
    return Result::SUCCESS;
}

nbl::Result nbl::BufferStream::_read(void* impl, void* data, usize length, usize* bytesRead) {

    nbl::BufferStream* reader = static_cast<nbl::BufferStream*>(impl);

    if(reader->_mReaderIndex >= reader->_mBuffer.length) {
        if(bytesRead) *bytesRead = 0;
        return nbl::result::END_OF_FILE;
    }

    usize remaining = reader->_mBuffer.length - reader->_mReaderIndex;
    usize toRead = length < remaining ? length : remaining;
    memcpy(data, reader->_mBuffer.data + reader->_mReaderIndex, toRead);
    reader->_mReaderIndex += toRead;
    if(bytesRead) {
        *bytesRead = toRead;
    }
    return Result::SUCCESS;
}
