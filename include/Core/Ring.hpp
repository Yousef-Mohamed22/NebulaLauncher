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
