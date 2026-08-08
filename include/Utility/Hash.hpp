#pragma once

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Core/Span.hpp>

namespace nbl {

nbl::Result fileSha1(nbl::StringView filePath, Span<u8> outBytes);

}
