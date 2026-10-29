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
 * \file Core/Stream.cpp
 * \author jwall64
 * \brief Implementation of Core/Stream.hpp
 */


#include <Core/Stream.hpp>
#include <Core/Result.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>

#include <cstring>
#include <cmath>

nbl::Result nbl::BufferStream::_write(void* impl, const void* data, usize length, usize* bytesWritten) {

    nbl::BufferStream* writer = static_cast<nbl::BufferStream*>(impl);

    if(writer->_mWriterIndex + length > writer->_mBuffer.length()) {
        return nbl::Result::OUT_OF_MEMORY;
    }
    memcpy(writer->_mBuffer.data() + writer->_mWriterIndex, data, length);
    writer->_mWriterIndex += length;
    if(bytesWritten) {
        *bytesWritten = length;
    }
    return Result::SUCCESS;
}

nbl::Result nbl::BufferStream::_read(void* impl, void* data, usize length, usize* bytesRead) {

    nbl::BufferStream* reader = static_cast<nbl::BufferStream*>(impl);

    if(reader->_mReaderIndex >= reader->_mBuffer.length()) {
        if(bytesRead) *bytesRead = 0;
        return nbl::Result::END_OF_STREAM;
    }

    usize remaining = reader->_mBuffer.length() - reader->_mReaderIndex;
    usize toRead = length < remaining ? length : remaining;
    memcpy(data, reader->_mBuffer.data() + reader->_mReaderIndex, toRead);
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



nbl::Result nbl::Writer::writeString(StringView str) {
    return writeAll(str.data(), str.length());
}

nbl::Result nbl::writeInt(i64 value, Writer writer) {

    if(value < 0) {
        NBL_CHECK(writer.writeChar('-'));
        value = -value;
    }

    return writeUint(static_cast<u64>(value), writer);
}

nbl::Result nbl::writeUint(u64 value, Writer writer) {

    if(value == 0) {
        return writer.writeChar('0');
    }

    u8 buffer[20];
    u8 index = 20;

    while(value > 0) {
        u8 lastDigit = value % 10;
        buffer[--index] = '0' + lastDigit;
        value /= 10;
    }

    return writer.writeAll(buffer + index, 20 - index);
}

nbl::Result nbl::writeFloat(f64 value, Writer writer, u8 precision) {

    static constexpr StringView nan("NaN");
    static constexpr StringView inf("Infinity");

    if(value < 0) {
        char c = '-';
        NBL_CHECK(writer.writeAll(&c, 1));
        value = -value;
    }

    i32 fpInfo = std::fpclassify(value);
    if(fpInfo == FP_INFINITE) {
        return writer.writeString(inf);
    } else if(fpInfo == FP_NAN) {
        return writer.writeString(nan);
    }

    u64 wholePart = static_cast<u64>(value);
    f64 fractionalPart = value - static_cast<f64>(wholePart);

    NBL_CHECK(writeUint(wholePart, writer));
    NBL_CHECK(writer.writeChar('.'));

    if(fractionalPart == 0) {
        return writer.writeChar('0');
    }

    precision = nbl::min(precision, static_cast<u8>(15));

    u8 buffer[15];
    u8 digit = 0;
    u8 index = 0;
    u8 lastNonZeroIndex = 0;

    while(fractionalPart > 0 && index < precision) {
        fractionalPart *= 10;
        digit = static_cast<u8>(fractionalPart);

        buffer[index++] = '0' + digit;
        if(digit != 0) {
            lastNonZeroIndex = index;
        }

        fractionalPart -= digit;
    }

    if(lastNonZeroIndex == 0) {
        return writer.writeChar('0');
    } else {
        return writer.writeAll(buffer, lastNonZeroIndex);
    }
}

nbl::Result nbl::writeHex(u64 value, Writer writer, u8 minDigits, bool lower) {
 
    if(minDigits > 16) return nbl::Result::INVALID_ARGUMENT;

    u8 buffer[16]{};
    u8 index = 16;

    char letterOffset = lower ? 'a' : 'A';

    while(value > 0) {
        u8 lastDigit = value & 0xF;
        if(lastDigit > 9) {
            buffer[--index] = letterOffset + (lastDigit - 10);
        } else {
            buffer[--index] = '0' + lastDigit;
        }
        value >>= 4;
    }
    while((16 - index) < minDigits) {
        buffer[--index] = '0';
    }

    return writer.writeAll(buffer + index, 16 - index);

}


nbl::Result nbl::write(HexEncoder encoder, Writer writer) {

    char numberOffset = '0';
    char letterOffset = (encoder.upper ? 'A' : 'a') - 10;
    for(u8 byte : encoder.data) {

        u8 n1 = (byte >> 4) & 0xF;
        u8 n2 = byte & 0xF;

        char hex[2] = {
            static_cast<char>(n1 + (n1 < 10) * numberOffset + (n1 >= 10) * letterOffset),
            static_cast<char>(n2 + (n2 < 10) * numberOffset + (n2 >= 10) * letterOffset)
        };
        NBL_CHECK(writer.writeAll(hex, 2));
    }
    return nbl::Result::SUCCESS;
}
