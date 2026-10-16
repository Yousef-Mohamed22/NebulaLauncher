#pragma once

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Core/Span.hpp>

namespace nbl {

nbl::Result fileSha1(nbl::StringView filePath, Span<u8> outBytes);
nbl::Result fileSha256(nbl::StringView filePath, Span<u8> outBytes);

nbl::Result sha1(Reader reader, Span<u8> outBytes);

nbl::Result sha256(Reader reader, Span<u8> outBytes);
nbl::Result sha256(ConstSpan<u8> bytes, Span<u8> outBytes);

nbl::Result md5(Reader reader, Span<u8> outBytes);
nbl::Result md5(ConstSpan<u8> bytesr, Span<u8> outBytes);

}
