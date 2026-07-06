// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Utf8.hpp
 * \author jwall64
 * \brief Utility functions for working with UTF-8 encoded strings.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Types.hpp>
#include <Core/Stream.hpp>

namespace nbl {

/**
 * \brief Encodes the given Unicode code point to a writer in UTF-8 format
 * \param writer The writer to write the code point to
 * \param codePoint The code point to encode
 * \return A result indicating the success or failure of the operation
 */
nbl::Result writeCodePoint(Writer writer, u32 codePoint);

}

