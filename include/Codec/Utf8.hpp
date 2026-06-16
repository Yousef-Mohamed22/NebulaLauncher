// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Utf8.hpp
 * \author jwall64
 * \brief Utility functions for working with UTF-8 encoded strings, including functions for encoding and decoding Unicode code points, as well as validating UTF-8 sequences.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Types.hpp>
#include <Core/Stream.hpp>

namespace nbl {

nbl::Result writeCodePoint(Writer writer, u32 codePoint);

}

