// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Time.hpp
 * \author jwall64
 * \brief A simple header for time-related functions, such as retrieving the current time in milliseconds since the Unix epoch.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/Result.hpp>

namespace nbl {

/**
 * \brief Returns the current time in milliseconds since the Unix epoch (January 1, 1970). This function provides a platform-independent way to retrieve the current time for use in timing operations, logging, or other time-related functionality.
 * \return The current time in milliseconds since the Unix epoch.
 */
u64 getCurrentTime();

/**
 * \brief Computes the time in milliseconds since the Unix epoch (January 1, 1970) given the current time in human-usable components.
 * \param year The year AD (1970-9999)
 * \param month The month of the year (1-12)
 * \param day The day of the month (1-31)
 * \param hour The hour of the day (0-23)
 * \param minute The minute of the hour (0-59)
 * \param second The second of the minute (0-59)
 * \param out The location to store the computed timestamp
 * \return The result of the operation
 */
Result utcFromComponents(u32 year, u8 month, u8 day, u8 hour, u8 minute, u8 second, u64* out);

}
