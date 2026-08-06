#include "Util.hpp"

nbl::Result nbl::mc::parseHex(StringView hex, Span<u8> out) {
    if(hex.length % 2 != 0) {
        return nbl::Result::INVALID_ARGUMENT;
    }
    if(hex.length / 2 > out.length) {
        return nbl::Result::INVALID_ARGUMENT;
    }

    for(usize i = 0 ; i < hex.length ; i++) {
        char hexChar = hex[i];
        if(!(hexChar >= '0' && hexChar <= '9')
        && !(hexChar >= 'a' && hexChar <= 'f')) {
            return nbl::Result::FORMAT_ERROR;
        }

        u8 hexValue = (hexChar >= '0' && hexChar <= '9') 
            ? hexChar - '0'
            : hexChar - 'a' + 10;

        usize byteIndex = i / 2;
        if(i % 2 == 0) {
            out[byteIndex] |= (hexValue << 4);
        } else {
            out[byteIndex] |= (hexValue & 0xF);
        }
    }

    return nbl::Result::SUCCESS;
}
