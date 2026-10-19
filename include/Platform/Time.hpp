/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file Platform/Time.hpp
 * \author jwall64
 * \brief Functions related to wall-clock time
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
