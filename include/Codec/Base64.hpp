#pragma once

#include <Core/Result.hpp>
#include <Core/Span.hpp>

namespace nbl {

struct Base64CharacterSet {

    const char* encodingTable;
    const char* decodingTable;

    static Base64CharacterSet STANDARD;
    static Base64CharacterSet URL_SAFE;
};

usize b64EncodedLength(usize length);
usize b64DecodedLength(usize length);

Result encodeBase64(Span<u8> input, Span<char> output, const Base64CharacterSet& characterSet = Base64CharacterSet::STANDARD);
Result decodeBase64(Span<char> input, Span<u8> output, const Base64CharacterSet& characterSet = Base64CharacterSet::STANDARD);


}
