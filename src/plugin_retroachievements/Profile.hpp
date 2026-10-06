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
