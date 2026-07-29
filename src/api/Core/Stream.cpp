// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Stream.cpp
 * \author jwall64
 * \brief Implementation for Core/Stream.hpp
 */

#include <Core/Stream.hpp>
#include <Core/Result.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>

#include <cstring>

nbl::Result nbl::BufferStream::_write(void* impl, const void* data, usize length, usize* bytesWritten) {

    nbl::BufferStream* writer = static_cast<nbl::BufferStream*>(impl);

    if(writer->_mWriterIndex + length > writer->_mBuffer.length) {
        return nbl::Result::OUT_OF_MEMORY;
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
        return nbl::Result::END_OF_STREAM;
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

nbl::Result nbl::BufferedWriter::flush() {
    NBL_CHECK(_mSink.writeAll(_pBuffer, _mPos));
    _mPos = 0;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::BufferedWriter::write(const void* data, usize length, usize* bytesWritten) {

    usize bufRemaining = _mBufLen - _mPos;
    usize index = 0;
    const u8* bytes = static_cast<const u8*>(data);

    while(length > bufRemaining) {
        memcpy(_pBuffer + _mPos, bytes + index, bufRemaining);
        _mPos += bufRemaining;
        length -= bufRemaining;
        index += bufRemaining;

        NBL_CHECK(flush());
        bufRemaining = _mBufLen;
    }

    if(length > 0) {
        memcpy(_pBuffer + _mPos, bytes + index, length);
        index += length;
        _mPos += length;
    }

    if(bytesWritten) *bytesWritten = index;
    return nbl::Result::SUCCESS;

}

nbl::Result nbl::BufferedReader::read(void* data, usize length, usize* bytesRead) {

    usize remaining = _mAvailable - _mPos;
    usize index = 0;
    u8* bytes = static_cast<u8*>(data);

    if(length <= remaining) {
        memcpy(bytes, _pBuffer + _mPos, length);
        _mPos += length;

        if(bytesRead) *bytesRead = length;
        return nbl::Result::SUCCESS;
    }

    if(remaining > 0) {
        memcpy(bytes, _pBuffer + _mPos, remaining);
        length -= remaining;
        index += remaining;
    }

    _mAvailable = 0;
    _mPos = 0; 

    if(length > _mBufLen) {
        
        usize justRead = 0;
        nbl::Result out = _mSource.read(static_cast<u8*>(data) + remaining, length, &justRead);
        if(out.error()) return out;

        index += justRead;
        if(bytesRead) *bytesRead = index;

        return out;
    }

    nbl::Result res = _mSource.read(_pBuffer, _mBufLen, &_mAvailable);
    if(res.error()) return res;

    usize toCopy = nbl::min(_mAvailable, length);
    memcpy(bytes + index, _pBuffer, toCopy);

    _mPos += toCopy;
    index += toCopy;
    
    if(bytesRead) *bytesRead = index;

    if(res == nbl::Result::END_OF_STREAM && _mAvailable > length) return nbl::Result::SUCCESS;
    return res;
}

