/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file Core/RingBuffer.cpp
 * \author jwall64
 * \brief Implementation of Core/RingBuffer.hpp
 */

#include <Core/Ring.hpp>

#include <Core/Types.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>

#include <cstring>

nbl::RingBuffer::~RingBuffer() {
    _pBuffer = nullptr;
    _mLength = 0;
    _mStart = 0;
    _mReaderIndex = 0;
    _mWriterIndex = 0;
}

nbl::Result nbl::RingBuffer::write(const void* data, usize dataLength, usize* bytesWritten) {

    if(_mReaderIndex > _mWriterIndex) {

        usize toWrite = nbl::min(_mReaderIndex - _mWriterIndex, dataLength);
        if(toWrite > 0) {
            memcpy(_pBuffer + _mWriterIndex, data, toWrite);
        }

        _mReaderIndex += toWrite;
        if(bytesWritten) *bytesWritten = toWrite;

    } else {
        usize remainingInBuffer = (_mLength - _mWriterIndex);
        if(dataLength <= remainingInBuffer) {

            if(dataLength > 0) {
                memcpy(_pBuffer + _mWriterIndex, data, dataLength);
                _mWriterIndex = _mWriterIndex + dataLength;
            }

            if(bytesWritten) *bytesWritten = dataLength;
            return nbl::Result::SUCCESS;
        }

        if(remainingInBuffer > 0) {
            memcpy(_pBuffer + _mWriterIndex, data, remainingInBuffer);
        }

        usize remainingToWrite = dataLength - remainingInBuffer;
        usize toWrite = nbl::min(remainingToWrite, _mReaderIndex);
        if(toWrite > 0) {
            memcpy(_pBuffer, static_cast<const u8*>(data) + remainingInBuffer, toWrite);
        }
        _mWriterIndex = toWrite;
        _mStart = _mWriterIndex;

        if(bytesWritten) *bytesWritten = toWrite + remainingInBuffer;
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::RingBuffer::read(void* buffer, usize bufferLength, usize* bytesRead) {
    return _peek(buffer, bufferLength, bytesRead, &_mReaderIndex);
}

nbl::Result nbl::RingBuffer::peek(void* buffer, usize bufferLength, usize* bytesRead) const {
    return _peek(buffer, bufferLength, bytesRead, nullptr);
}

nbl::Result nbl::RingBuffer::_peek(void* buffer, usize bufferLength, usize* bytesRead, usize* newReaderIndex) const {

    usize peekIndex = _mReaderIndex;
    if(_mWriterIndex < _mReaderIndex) {
        usize readable = (_mLength - _mReaderIndex) + _mWriterIndex;
        usize toRead = nbl::min(readable, bufferLength);

        if(toRead > 0) {
            usize remainingInBuffer = _mLength - _mReaderIndex;

            usize toReadFromEnd = nbl::min(toRead, remainingInBuffer);
            memcpy(buffer, _pBuffer + _mReaderIndex, nbl::min(toRead, remainingInBuffer));
            peekIndex = _mReaderIndex + toReadFromEnd;

            if(toRead > remainingInBuffer) {
                usize toReadFromBeginning = nbl::min(_mWriterIndex, toRead - remainingInBuffer);
                memcpy(static_cast<u8*>(buffer) + remainingInBuffer, _pBuffer, toReadFromBeginning);
                peekIndex = toReadFromBeginning;
            }
        }

        if(bytesRead) *bytesRead = toRead;

    } else {
        usize readable = _mWriterIndex - _mReaderIndex;
        usize toRead = nbl::min(readable, bufferLength);

        if(toRead > 0) {
            memcpy(buffer, _pBuffer + _mReaderIndex, toRead);
        }
        peekIndex += toRead;

        if(bytesRead) *bytesRead = toRead;
    }

    if(newReaderIndex) *newReaderIndex = peekIndex;
    return nbl::Result::SUCCESS;

}

nbl::Result nbl::RingBuffer::overwrite(const void* data, usize dataLength) {

    usize toWrite = nbl::min(dataLength, _mLength);
    _mReaderIndex = (_mWriterIndex + toWrite) % _mLength;
    return write(static_cast<const u8*>(data) + (dataLength - toWrite), toWrite, nullptr);
}

nbl::Result nbl::RingBuffer::create(void* buffer, usize length, nbl::RingBuffer* out) {

    out->_pBuffer = static_cast<u8*>(buffer);
    out->_mLength = length;
    
    out->_mWriterIndex = 0;
    out->_mReaderIndex = 0;

    return nbl::Result::SUCCESS;
}
