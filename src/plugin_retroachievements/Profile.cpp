#include "Profile.hpp"
#include "Constants.hpp"

#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>
#include <Platform/StringBuilder.hpp>

nbl::Result nbl::ra::downloadProfile(HttpClient *client, StringView apiKey, StringView username, LinearAllocator *storage, UserProfile *out) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

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
