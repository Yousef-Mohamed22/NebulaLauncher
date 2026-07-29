// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Stream.hpp
 * \author jwall64
 * \brief A simple implementation of stream reader and writer interfaces.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/Span.hpp>

namespace nbl {

struct StringView;

/**
 * \brief A simple stream writer interface that can write data to an underlying stream.
 */
struct Writer {

    /**
     * \brief Writes data to the stream.
     * \param data Pointer to the data to be written.
     * \param length The number of bytes to write.
     * \param bytesWritten Pointer to a variable that will receive the number of bytes actually written. Can be nullptr.
     * \return A Result indicating success or failure of the write operation.
     */
    Result write(const void* data, usize length, usize* bytesWritten) {
        return pfnWrite(impl, data, length, bytesWritten);
    }

    /**
     * \brief Writes all data to the stream, ensuring that the entire buffer is written before returning.
     * \param data Pointer to the data to be written.
     * \param length The number of bytes to write.
     * \return A Result indicating success or failure of the write operation.
     */
    Result writeAll(const void* data, usize length) {
        usize totalBytesWritten = 0;
        while (totalBytesWritten < length) {
            usize bytesWritten;
            Result result = write(static_cast<const char*>(data) + totalBytesWritten, length - totalBytesWritten, &bytesWritten);
            if (result.error()) {
                return result;
            }
            totalBytesWritten += bytesWritten;
        }
        return Result::SUCCESS;
    }

    /**
     * \brief Writes a string to the stream.
     * \param str The string to be written.
     * \return A Result indicating success or failure of the write operation.
     */
    Result writeString(StringView str);

    /**
     * \brief Writes a single character to the stream.
     * \param c The character to be written.
     * \return A Result indicating success or failure of the write operation.
     */
    Result writeChar(char c) {
        return writeAll(&c, 1);
    }

    /**
     * \brief An opaque handle to the underlying stream implementation.
     */
    void* impl;

    /**
     * \brief Function pointer to the implementation of the write operation for the underlying stream.
     */
    Result (*pfnWrite)(void* impl, const void* data, usize length, usize* bytesWritten);
};

/**
 * \brief A simple stream reader interface that can read data from an underlying stream.
 */
struct Reader {

    /**
     * \brief Reads data from the stream.
     * \param data Pointer to a buffer that will receive the read data.
     * \param length The maximum number of bytes to read.
     * \param bytesRead Pointer to a variable that will receive the number of bytes actually read. Can be nullptr.
     * \return A Result indicating success or failure of the read operation.
     */
    Result read(void* data, usize length, usize* bytesRead) {
        return pfnRead(impl, data, length, bytesRead);
    }

    /**
     * \brief Reads data from the stream, ensuring that the entire buffer is filled before returning.
     * \param data Pointer to a buffer that will receive the read data.
     * \param length The number of bytes to read.
     * \return A Result indicating success or failure of the read operation.
     */
    Result readAll(void* data, usize length) {
        usize totalBytesRead = 0;
        while (totalBytesRead < length) {
            usize bytesRead;
            Result result = read(static_cast<char*>(data) + totalBytesRead, length - totalBytesRead, &bytesRead);
            if (result.error()) {
                return result;
            } else if(bytesRead == 0 && result == nbl::Result::END_OF_STREAM) {
                return nbl::Result::UNEXPECTED_END_OF_STREAM;
            }
            totalBytesRead += bytesRead;
        }
        return Result::SUCCESS;
    }

    /**
     * \brief An opaque handle to the underlying stream implementation.
     */
    void* impl;

    /**
     * \brief Function pointer to the implementation of the read operation for the underlying stream.
     */
    Result (*pfnRead)(void* impl, void* data, usize length, usize* bytesRead);
};

/**
 * \brief A simple stream implementation for reading and writing from a known buffer.
 */
class BufferStream {
public:

    BufferStream(void* buffer, usize length) :
        _mBuffer{static_cast<u8*>(buffer), length},
        _mReaderIndex(0),
        _mWriterIndex(0)
    { }

    operator Writer() {
        return Writer{ this, _write };
    }

    operator Reader() {
        return Reader{ this, _read };
    }

    usize writerIndex() const { return _mWriterIndex; }

    usize readerIndex() const { return _mReaderIndex; }

    void resetWriterIndex() { _mWriterIndex = 0; }

    void resetReaderIndex() { _mReaderIndex = 0; }

private:
    nbl::Span<u8> _mBuffer;
    usize _mReaderIndex;
    usize _mWriterIndex;
    
    static Result _write(void* impl, const void* data, usize length, usize* bytesWritten);
    
    static Result _read(void* impl, void* data, usize length, usize* bytesRead);

};

/**
 * \brief A dummy stream that disards bytes written to it and is always at the end of file when reading
 */
class NullStream {
public:

    operator Writer() {
        return Writer{ nullptr, _write };
    }

    operator Reader() {
        return Reader{ nullptr, _read };
    }

private:

    static Result _write(void* impl, const void* data, usize length, usize* bytesWritten) {
        if(bytesWritten) {
            *bytesWritten = length;
        }
        return Result::SUCCESS;
    }

    static Result _read(void* impl, void* data, usize length, usize* bytesRead) {
        if(bytesRead) {
            *bytesRead = 0;
        }
        return nbl::Result::END_OF_STREAM;
    }
};

/**
 * \brief A writer which first writes to a buffer in memory, then flushes data to the underlying writer in batches
 */
class BufferedWriter {
public:

    BufferedWriter(Writer sink, void* buffer, usize bufLen) :
        _mSink(sink),
        _mPos(0),
        _mBufLen(bufLen),
        _pBuffer(static_cast<u8*>(buffer))
    { }

    ~BufferedWriter() {
        flush();
    }

    operator Writer() { 
        return Writer{this, _write};
    }

    Result flush();

    Result write(const void* data, usize length, usize* bytesWritten);

    Result writeAll(const void* data, usize length) {
        return operator Writer().writeAll(data, length);
    }

private:
    static Result _write(void* impl, const void* data, usize length, usize* bytesWritten) {
        BufferedWriter* writer = static_cast<BufferedWriter*>(impl);
        return writer->write(data, length, bytesWritten);
    }

    Writer _mSink;
    usize _mPos;
    usize _mBufLen;
    u8* _pBuffer;
};

/**
 * \brief A writer which batch reads from some underlying reader into to a buffer in memory 
 */
class BufferedReader {
public:

    BufferedReader(Reader source, void* buffer, usize bufLen) :
        _mSource(source),
        _mPos(0),
        _mBufLen(bufLen),
        _pBuffer(static_cast<u8*>(buffer))
    { }

    operator Reader() { 
        return Reader{ this, _read };
    }

    Result read(void* data, usize length, usize* bytesRead);

    inline Result readAll(void* data, usize length) {
        return operator Reader().readAll(data, length);
    }

private:
    static Result _read(void* impl, void* data, usize length, usize* bytesRead) {
        BufferedReader* reader = static_cast<BufferedReader*>(impl);
        return reader->read(data, length, bytesRead);
    }

    Reader _mSource;
    usize _mPos;
    usize _mAvailable;
    usize _mBufLen;
    u8* _pBuffer;
};

}
