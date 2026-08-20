#pragma once

#include <Core/Types.hpp>
#include <Core/Span.hpp>
#include <Core/Result.hpp>

namespace nbl {


Result secureRandom(Span<u8> buffer);


}
