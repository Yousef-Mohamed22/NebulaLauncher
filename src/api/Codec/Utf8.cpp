#include <Codec/Utf8.hpp>


namespace {

constexpr static u8 _PFX_TWO_BYTE = 0b11000000;
constexpr static u8 _PFX_THREE_BYTE = 0b11100000;
constexpr static u8 _PFX_FOUR_BYTE = 0b11110000; 
constexpr static u8 _PFX_CONTINUATION = 0b10000000;

constexpr static u8 _MASK_TWO_BYTE = 0b00011111;
constexpr static u8 _MASK_THREE_BYTE = 0b00001111;
constexpr static u8 _MASK_FOUR_BYTE = 0b00000111;
constexpr static u8 _MASK_CONTINUATION = 0b00111111;

constexpr static u32 _MAX_ONE_BYTE = 0x7F;
constexpr static u32 _MAX_TWO_BYTE = 0x7FF;
constexpr static u32 _MAX_THREE_BYTE = 0xFFFF;
constexpr static u32 _MAX_FOUR_BYTE = 0x10FFFF;

}


nbl::Result nbl::writeCodePoint(nbl::Writer writer, u32 codePoint) {
    
    if(codePoint <= _MAX_ONE_BYTE) {
        char byte = static_cast<char>(codePoint);
        return writer.writeAll(&byte, 1);
    } else if(codePoint <= _MAX_TWO_BYTE) {
        char bytes[2];
        bytes[0] = static_cast<char>(_PFX_TWO_BYTE | (codePoint >> 6));
        bytes[1] = static_cast<char>(_PFX_CONTINUATION | (codePoint & _MASK_CONTINUATION));
        return writer.writeAll(bytes, 2);
    } else if(codePoint <= _MAX_THREE_BYTE) {
        char bytes[3];
        bytes[0] = static_cast<char>(_PFX_THREE_BYTE | (codePoint >> 12));
        bytes[1] = static_cast<char>(_PFX_CONTINUATION | ((codePoint >> 6) & _MASK_CONTINUATION));
        bytes[2] = static_cast<char>(_PFX_CONTINUATION | (codePoint & _MASK_CONTINUATION));
        return writer.writeAll(bytes, 3);
    } else if(codePoint <= _MAX_FOUR_BYTE) {
        char bytes[4];
        bytes[0] = static_cast<char>(_PFX_FOUR_BYTE | (codePoint >> 18));
        bytes[1] = static_cast<char>(_PFX_CONTINUATION | ((codePoint >> 12) & _MASK_CONTINUATION));
        bytes[2] = static_cast<char>(_PFX_CONTINUATION | ((codePoint >> 6) & _MASK_CONTINUATION));
        bytes[3] = static_cast<char>(_PFX_CONTINUATION | (codePoint & _MASK_CONTINUATION));
        return writer.writeAll(bytes, 4);
    } else {
        return nbl::Result::INVALID_ARGUMENT;
    }
    
}
