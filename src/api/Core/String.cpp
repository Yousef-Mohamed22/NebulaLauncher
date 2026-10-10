// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file String.cpp
 * \author jwall64
 * \brief Implementation for Core/String.hpp
 */

#include <Core/String.hpp>
#include <Core/Math.hpp>

#include <Platform/Logging.hpp>
#include <Platform/Assert.hpp>

#include <cmath>

nbl::Result nbl::Writer::writeString(StringView str) {
    return writeAll(str.data, str.length);
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


nbl::Result nbl::toString(HexEncoder encoder, Writer writer) {

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
