// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Time.hpp
 * \author jwall64
 * \brief A simple header for time-related functions, such as retrieving the current time in milliseconds since the Unix epoch.
 */

#pragma once

#include <Core/Types.hpp>

namespace nbl {

/**
 * \brief Returns the current time in milliseconds since the Unix epoch (January 1, 1970). This function provides a platform-independent way to retrieve the current time for use in timing operations, logging, or other time-related functionality.
 * \return The current time in milliseconds since the Unix epoch.
 */
u64 getCurrentTime();

}
