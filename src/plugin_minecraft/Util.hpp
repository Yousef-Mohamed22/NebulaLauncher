#pragma once

#include <Core/Result.hpp>
#include <Core/Span.hpp>
#include <Core/String.hpp>

namespace nbl::mc {

Result parseHex(StringView hex, Span<u8> out);

}
