#include <gtest/gtest.h>

#include <Core/Stream.hpp>
#include <Core/Types.hpp>
#include <Core/Result.hpp>
#include <Core/Math.hpp>

TEST(Stream, BufferedReader) {

    u8 dataToRead[4500] = {};
    for(auto i = 0 ; i < 4500 ; i++) {
        dataToRead[i] = static_cast<u8>(i % 256);
    }

    u8 buffer[1024] = {};

    nbl::BufferStream stream(dataToRead, 4500);
    nbl::BufferedReader reader(stream, buffer, 1024);

    u8 readBuffer[600] = {};
    usize bytesRead = 0;
    usize index = 0;

    do {
        nbl::Result res = reader.read(readBuffer, 600, &bytesRead);
        ASSERT_FALSE(res.error());
        ASSERT_LE(bytesRead, 600);
 
        for(auto i = 0 ; i < bytesRead ; i++) {
            ASSERT_EQ(readBuffer[i], static_cast<u8>(index % 256));
            index++;
        }

        if(res == nbl::Result::END_OF_STREAM) break;
    } while(true);

    ASSERT_EQ(index, 4500);
}

TEST(Stream, BufferedWriter) {

    u8 dataToWrite[4500] = {};
    for(auto i = 0 ; i < 4500 ; i++) {
        dataToWrite[i] = static_cast<u8>(i % 256);
    }

    u8 buffer[1024] = {};
    u8 writtenData[4500] = {};

    nbl::BufferStream stream(writtenData, 4500);
    nbl::BufferedWriter writer(stream, buffer, 1024);

    usize bytesWritten = 0;
    usize index = 0;

    do {

        usize remaining = 4500 - index;
        if(remaining == 0) break;

        nbl::Result res = writer.write(dataToWrite + index, nbl::min(static_cast<usize>(600), remaining), &bytesWritten);
        ASSERT_FALSE(res.error());
        ASSERT_LE(bytesWritten, 600);

        index += bytesWritten;

        if(res == nbl::Result::END_OF_STREAM) break;
    } while(true);

    writer.flush();

    ASSERT_EQ(index, 4500);
    for(auto i = 0 ; i < index ; i++) {
        ASSERT_EQ(writtenData[i], static_cast<u8>(i % 256));
    }

}
