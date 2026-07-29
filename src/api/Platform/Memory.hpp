// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Memory.hpp
 * \author jwall64
 * \brief Platform-specific memory management functions.
 */

#pragma once

#include <Core/Result.hpp>

namespace nbl {

usize pageSize() noexcept;

Result mapMemory(usize* size, void** outPtr) noexcept;

Result unmapMemory(void* ptr, usize* size) noexcept;

Result commitMemory(void* ptr, usize* size) noexcept;

Result uncommitMemory(void* ptr, usize* size) noexcept;

}
