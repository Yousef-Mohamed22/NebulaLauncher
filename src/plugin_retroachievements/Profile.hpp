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
 * \file Profile.hpp
 * \author jwall64
 * \brief RetroAchievements account management
 */

#pragma once

#include <Core/String.hpp>
#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>
#include <Network/HttpClient.hpp>

namespace nbl::ra {


struct UserProfile {
    nbl::StringView ulid;
    nbl::StringView apiKey;
    nbl::StringView username;
    nbl::StringView pfp;
    u64 points;
    u64 truePoints;

    bool isValid() {
        return !ulid.empty() && !apiKey.empty();
    }
};


Result downloadProfile(HttpClient* client, StringView apiKey, StringView username, LinearAllocator* storage, UserProfile* out);

}
