// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Memory.hpp
 * \author jwall64
 * \brief Platform-specific memory management functions.
 */

#pragma once

#include <Core/Result.hpp>

namespace nbl::memory {

extern usize gPageSize;

nbl::Result mapMemory(void** outPtr, usize* size);
nbl::Result unmapMemory(void* ptr, usize* size);

nbl::Result commitMemory(void* ptr, usize* size);
nbl::Result decommitMemory(void* ptr, usize* size);

}
