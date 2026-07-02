#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>

namespace nbl {


class RingBuffer {
public:

    RingBuffer() = default;
    ~RingBuffer();

    Result write(const void* data, usize dataLength, usize* bytesWritten);
    Result read(void* buffer, usize bufferLength, usize* bytesRead);

    Result overwrite(const void* data, usize dataLength);

    inline void rewind() {
        _mReaderIndex = _mStart;
    }

    static Result create(void* buffer, usize length, RingBuffer* out);

private:

    u8* _pBuffer;
    usize _mLength;

    usize _mStart;
    usize _mWriterIndex;
    usize _mReaderIndex;

};


}
