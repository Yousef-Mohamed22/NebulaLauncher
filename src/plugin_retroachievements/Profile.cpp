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
 * \file Profile.cpp
 * \author jwall64
 * \brief Implementation of Profile.hpp
 */

#include "Profile.hpp"
#include "Constants.hpp"

#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>
#include <Utility/StringBuilder.hpp>

nbl::Result nbl::ra::downloadProfile(HttpClient *client, StringView apiKey, StringView username, LinearAllocator *storage, UserProfile *out) {

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());

    nbl::StringView url;
    nbl::StringBuilder(&frame).append(RA_API_URL).append("/API_GetUserProfile.php?u=")
        .append(nbl::UrlEncode(username))
        .append("&y=")
        .append(nbl::UrlEncode(apiKey))
        .buildNullTerminated(&url);

    char* end = static_cast<char*>(frame.end());
    NBL_CHECK(client->makeRequest(url, frame.writer()));

    u32 failures = 0;
    client->awaitAll(&failures);
    if(failures) return nbl::Result::IO_ERROR;

    StringView responseJson(end, static_cast<char*>(frame.end()) - end);
    JsonElement response;

    NBL_CHECK(nbl::parseJson(StringReader(responseJson), &frame, &response));

    if(response.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
    
    NBL_CHECK(nbl::StringBuilder(storage).append(apiKey).buildNullTerminated(&out->apiKey));
    NBL_CHECK(nbl::StringBuilder(storage).append(response.object.getString("User")).buildNullTerminated(&out->username));
    NBL_CHECK(nbl::StringBuilder(storage).append(response.object.getString("ULID")).buildNullTerminated(&out->ulid));
    NBL_CHECK(nbl::StringBuilder(storage).append(RA_MEDIA_URL).append(response.object.getString("UserPic")).buildNullTerminated(&out->pfp));

    out->points = response.object.getNumber("TotalPoints").asUint();
    out->truePoints = response.object.getNumber("TotalTruePoints").asUint();

    return nbl::Result::SUCCESS;
}
