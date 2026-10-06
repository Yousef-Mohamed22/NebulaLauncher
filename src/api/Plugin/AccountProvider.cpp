#include <Plugin/AccountProvider.hpp>
#include <Data/OnlineAccountModel.hpp>
#include <Platform/Time.hpp>
#include <Codec/Base64.hpp>
#include <Core/Random.hpp>
#include <Network/HttpClient.hpp>
#include <Network/HttpServer.hpp>
#include <Platform/Random.hpp>
#include <Utility/Hash.hpp>
#include "Plugin/Field.hpp"


void nbl::LoginTracker::complete(PluginContext* context, StringView username, StringView icon, bool iconNearestScaling, Span<FieldValue> values) {

    ArenaFrame frame(Arena::threadLocal());
    StringBuilder builder(&frame);
    nbl::Result res;
    if((res = nbl::encodeFieldValues(context, _pActive->fields, values, builder)).error()) {
        NBL_ERROR("Failed to encode field values for online account: ", res.value);
        _finishTracking(false, "Failed to encode field values.");
        return;
    }

    StringView fieldsStr;
    if((res = builder.buildNullTerminated(&fieldsStr)).error()) {
        NBL_ERROR("Failed to build fields string for online account: ", res.value);
        _finishTracking(false, "Failed to build fields string.");
        return;
    }

    nbl::OnlineAccount account={
        .id = _mAccount,
        .fields = fieldsStr,
        .provider = _pActive->id,
        .updatedAt = nbl::getCurrentTime(),
        .username = username,
        .icon = icon,
        .iconNearestScaling = iconNearestScaling,
        .status = OnlineAccountStatus::LOGGED_IN
    };

    res = OnlineAccount::update(context->databaseConnection(), account);

    if(res.success()) {
        _finishTracking(true, {});
    } else {
        _finishTracking(false, "Failed to save account info to database.");
    }
}

nbl::Result nbl::LoginTracker::beginOAuthLogin(StringView url, StringView clientId, Span<StringView> scopes, u16 port, OAuthResponseType type, StringBuilder* responseBuilder) {

    if(port == 0) {
        nbl::Random random{};
        random.seed(nbl::getCurrentTime());
        port = 40000 + (random.next() % 10000);
    } 

    for(const StringView& scope : scopes) {
        for(char c : scope) {
            if(c == ' ' || c == '"' || c == '\\') return nbl::Result::INVALID_ARGUMENT;
        }
    }

    // PKCE
    nbl::Array<u8,96> challengeBytes;
    nbl::Array<char, 128> challengeB64;
    NBL_CHECK(nbl::secureRandom(challengeBytes));
    NBL_CHECK(nbl::encodeBase64(challengeBytes, challengeB64, nbl::Base64CharacterSet::URL_SAFE));

    nbl::Array<u8,32> hash{};
    NBL_CHECK(nbl::sha256(nbl::BufferStream(challengeB64.span()), hash));

    nbl::Array<char, 43> hashB64{};
    NBL_CHECK(nbl::encodeBase64(hash, hashB64, nbl::Base64CharacterSet::URL_SAFE));

    {
        StringView responseTypeStr{};
        switch(type) {
            default:
            case nbl::OAuthResponseType::NONE:
                responseTypeStr = "none";
                break;
            case nbl::OAuthResponseType::CODE:
                responseTypeStr = "code";
                break;
            case nbl::OAuthResponseType::TOKEN:
                responseTypeStr = "token";
                break;
        }

        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        StringView finalUrl;
        nbl::StringBuilder builder(&frame);
        builder
            .append(url)
            .append("?client_id=").append(UrlEncode(clientId))
            .append("&code_challenge=").append(StringView(hashB64.data, hashB64.size()))
            .append("&code_challenge_method=S256")
            .append("&response_type=").append(responseTypeStr)
            .append("&redirect_uri=http%3A%2F%2Flocalhost%3A").append(port);

        usize index = 0;
        if(scopes.length > 0) {
            builder.append("&scope=");
            for(StringView& scope : scopes) {
                if(index++ > 0) {
                    builder.append("%20");
                }
                builder.append(UrlEncode(scope));
            }
        }
        NBL_CHECK(builder.build(&finalUrl));
        if(!_pfnOpenUrl(finalUrl, _pUserData)) {
            return nbl::Result::INVALID_STATE;
        }
    }

    NBL_CHECK(nbl::OAuthCallbackListener::listen(port, responseBuilder));

    return nbl::Result::SUCCESS;
}


