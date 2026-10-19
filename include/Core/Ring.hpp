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
 * \file Core/Ring.hpp
 * \author jwall64
 * \brief Ring and RingBuffer types
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>

#include <utility>
#include <new>

namespace nbl {


class RingBuffer {
public:

    RingBuffer() = default;
    ~RingBuffer();

    Result write(const void* data, usize dataLength, usize* bytesWritten);
    Result read(void* buffer, usize bufferLength, usize* bytesRead);
    Result peek(void* buffer, usize bufferLength, usize* bytesRead) const;

    Result overwrite(const void* data, usize dataLength);

    inline void rewind() {
        _mReaderIndex = _mStart;
    }

    static Result create(void* buffer, usize length, RingBuffer* out);

private:

    Result _peek(void* buffer, usize bufferLength, usize* bytesRead, usize* newReaderIndex) const;

    u8* _pBuffer;
    usize _mLength;

    usize _mStart;
    usize _mWriterIndex;
    usize _mReaderIndex;

    template<typename T, usize Length>
    friend class Ring;

};



template<typename T, usize Length>
class Ring {
public:

    bool bOverwrite;

    Result push(const T& value) noexcept {
        T* loc = nullptr;
        LO_CHECK(_alloc(bOverwrite, &loc));
        new(loc) T(value);
        return nbl::Result::SUCCESS;
    }
    Result push(T&& value) noexcept {
        T* loc = nullptr;
        LO_CHECK(_alloc(bOverwrite, &loc));
        new(loc) T(std::move(value));
        return nbl::Result::SUCCESS;
    }
    
    template<typename... Args>
    Result emplace(Args&&... args) noexcept {
        T* loc = nullptr;
        LO_CHECK(_alloc(bOverwrite, &loc));
        new(loc) T(std::forward<Args...>(args...));
        return nbl::Result::SUCCESS;

    }

    Result peek(const T** value) const noexcept {
        if(_mRingBuffer._mWriterIndex == 0) {
            return nbl::Result::NOT_FOUND;
        }
        else if(_mRingBuffer._mReaderIndex == _mRingBuffer._mLength) {
            *value = reinterpret_cast<const T*>(_mRingBuffer._pBuffer);
        } 
        else {
            *value = reinterpret_cast<const T*>(_mRingBuffer._pBuffer + _mRingBuffer._mReaderIndex);
        }
    }

    Result pop(T* value) noexcept {

        T* peeked = nullptr;
        LO_CHECK(peek(&peeked));

        u8 dummy[sizeof(T)] = {};
        usize read = 0;
        LO_CHECK(_mRingBuffer.read(dummy, sizeof(T), &read));
        if(read != sizeof(T)) return nbl::Result::UNEXPECTED_ERROR;
        
        *value = std::move(*peeked);
    }

private:

    Result _alloc(bool overwrite, T** location) {
        u8 dummy[sizeof(T)] = {};
        if(overwrite) {
            LO_CHECK(_mRingBuffer.overwrite(dummy, sizeof(T)));
        } else {
            usize written = 0;
            LO_CHECK(_mRingBuffer.write(dummy, sizeof(T), &written));
            if(written != sizeof(T)) return nbl::Result::OUT_OF_SPACE;
        }

        *location = reinterpret_cast<T*>(_mRingBuffer._pBuffer + _mRingBuffer._mWriterIndex - sizeof(T));
        return nbl::Result::SUCCESS;
    }

    u8 _mBuffer[sizeof(T) * Length];
    RingBuffer _mRingBuffer;

};

}
