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

nbl::Result nbl::ra::loadProfile(Reader reader, LinearAllocator* storage, UserProfile* out) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    JsonElement element{};
    NBL_CHECK(nbl::parseJson(reader, &frame, &element));
    if(element.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    NBL_CHECK(nbl::StringBuilder(storage).append(element.object.getString("api_key")).buildNullTerminated(&out->apiKey));
    NBL_CHECK(nbl::StringBuilder(storage).append(element.object.getString("username")).buildNullTerminated(&out->username));
    NBL_CHECK(nbl::StringBuilder(storage).append(element.object.getString("ulid")).buildNullTerminated(&out->ulid));
    NBL_CHECK(nbl::StringBuilder(storage).append(element.object.getString("pfp")).buildNullTerminated(&out->pfp));

    out->points = element.object.getNumber("points").asUint();
    out->truePoints = element.object.getNumber("true_points").asUint();

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::ra::saveProfile(Writer writer, const UserProfile& profile) {

    return nbl::JsonWriter(writer).writeObject([&](auto& obj) {
        obj.add("api_key", profile.apiKey);
        obj.add("username", profile.username);
        obj.add("ulid", profile.ulid);
        obj.add("pfp", profile.pfp);
        obj.add("points", profile.points);
        obj.add("true_points", profile.truePoints);
    });
}
