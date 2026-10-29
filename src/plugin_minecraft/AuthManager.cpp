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

#include "AuthManager.hpp"
#include "SkinUtil.hpp"

#include <Core/Math.hpp>
#include <Platform/Arena.hpp>
#include <Platform/File.hpp>
#include <Platform/Time.hpp>
#include <Codec/Json.hpp>
#include <Network/HttpClient.hpp>
#include <Data/OnlineAccountModel.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Utility/StringBuilder.hpp>

namespace {

nbl::Result _finishLogin(nbl::PluginContext* context, nbl::HttpClient& client, nbl::Span<nbl::FieldValue> values, nbl::StringView accessToken, nbl::StringView skinFolder, nbl::LoginTracker* tracker) {
    nbl::AllocationFrame frame(nbl::Arena::threadLocal());
    nbl::JsonElement response{};

    nbl::StringView refreshToken = values[3].stringValue.raw;

    {
        nbl::StringView tokenField;
        NBL_CHECK(nbl::StringBuilder(&frame).append("d=").append(accessToken).build(&tokenField));

        nbl::StringView payload;
        nbl::StringBuilder payloadBuilder(&frame);

        NBL_CHECK(nbl::JsonWriter(payloadBuilder)
            .writeObject([&](auto obj) { obj
                .writeObject("Properties", [&](auto props) { props
                    .add("AuthMethod", "RPS")
                    .add("SiteName", "user.auth.xboxlive.com")
                    .add("RpsTicket", tokenField);
                    return nbl::Result::SUCCESS;
                })
                .add("RelyingParty", "http://auth.xboxlive.com")
                .add("TokenType", "JWT");
                return nbl::Result::SUCCESS;
            }));
        NBL_CHECK(payloadBuilder.buildNullTerminated(&payload));

        nbl::Array headers = {
            nbl::HttpHeader { .name = "Content-Type", .value = "application/json" },
            nbl::HttpHeader { .name = "x-xbl-contract-version", .value = "1" }
        };

        void* outputBegin = frame.end();
        client.makePostRequest("https://user.auth.xboxlive.com/user/authenticate", payload, frame.writer(), { .headers = headers });

        u32 numFailures = 0;
        client.awaitAll(&numFailures);
        if(numFailures > 0) {
            NBL_WARN("Xbox auth request failed");
            return nbl::Result::IO_ERROR;
        }
        usize length = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(outputBegin);
        nbl::StringView str(static_cast<char*>(outputBegin), length);

        NBL_CHECK(nbl::parseJson(str, &frame, &response));
    }

    nbl::StringView liveToken = response.asObject().getString("Token");
    if(liveToken.length() == 0) {
        NBL_WARN("Xbox auth sent invalid token: ", response);
        return nbl::Result::FORMAT_ERROR;
    }

    tracker->postStatus("Requesting XSTS token");


    {
        nbl::StringView payload;
        nbl::StringBuilder payloadBuilder(&frame);

        NBL_CHECK(nbl::JsonWriter(payloadBuilder)
            .writeObject([&](auto obj) { obj
                .writeObject("Properties", [&](auto props) { props
                    .add("SandboxId", "RETAIL")
                    .writeArray("UserTokens", [&](auto tokens) { tokens
                        .add(liveToken);
                        return nbl::Result::SUCCESS;
                    });
                    return nbl::Result::SUCCESS;
                })
                .add("RelyingParty", "rp://api.minecraftservices.com/")
                .add("TokenType", "JWT");
                return nbl::Result::SUCCESS;
            }));
        NBL_CHECK(payloadBuilder.buildNullTerminated(&payload));


        nbl::Array headers = {
            nbl::HttpHeader { .name = "Content-Type", .value = "application/json" },
            nbl::HttpHeader { .name = "x-xbl-contract-version", .value = "1" }
        };

        void* outputBegin = frame.end();
        client.makePostRequest("https://xsts.auth.xboxlive.com/xsts/authorize", payload, frame.writer(), { .headers = headers });

        u32 numFailures = 0;
        client.awaitAll(&numFailures);
        if(numFailures > 0) {
            NBL_WARN("XSTS request failed");
            return nbl::Result::IO_ERROR;
        }
        usize length = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(outputBegin);
        nbl::StringView responseStr(static_cast<char*>(outputBegin), length);

        NBL_CHECK(nbl::parseJson(responseStr, &frame, &response));
    }

    nbl::StringView xstsToken = response.asObject().getString("Token");
    nbl::StringView uhs = response.asObject().getObject("DisplayClaims").getArray("xui").getObject(0).getString("uhs");

    if(xstsToken.length() == 0 || uhs.length() == 0) {
        NBL_WARN("XSTS send invalid response: ", response);
        return nbl::Result::FORMAT_ERROR;
    }

    tracker->postStatus("Requesting Minecraft access token");

    {
        nbl::StringView payload;
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append("{\"identityToken\": \"XBL3.0 x=")
                .append(uhs)
                .append(";")
                .append(xstsToken)
                .append("\"}")
                .build(&payload));

        nbl::Array headers = {
            nbl::HttpHeader { .name = "Content-Type", .value = "application/json" }
        };

        void* outputBegin = frame.end();
        client.makePostRequest("https://api.minecraftservices.com/authentication/login_with_xbox", payload, frame.writer(), { .headers = headers });

        u32 numFailures = 0;
        client.awaitAll(&numFailures);
        if(numFailures > 0) {
            NBL_WARN("Minecraft auth request failed");
            return nbl::Result::IO_ERROR;
        }

        usize length = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(outputBegin);
        nbl::StringView responseStr(static_cast<char*>(outputBegin), length);

        nbl::StringView res(static_cast<char*>(outputBegin), length);
        NBL_DEBUG("POST request complete: ", res);

        NBL_CHECK(nbl::parseJson(responseStr, &frame, &response));
    }

    u64 expiry = (response.asObject().getNumber("expires_in").asUint() * 1000) + nbl::getCurrentTime();

    nbl::StringView mcAccessToken = response.asObject().getString("access_token");
    if(mcAccessToken.length() == 0) {
        NBL_WARN("Minecraft auth sent zero-length access token");
        return nbl::Result::FORMAT_ERROR;
    }

    tracker->postStatus("Requesting Minecraft profile");
    {
        nbl::StringView tokenField;
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append("Bearer ")
                .append(mcAccessToken)
                .build(&tokenField));

        void* outputBegin = frame.end();

        nbl::Array headers = {
            nbl::HttpHeader { .name = "Authorization", .value = tokenField }
        };

        client.makeRequest("https://api.minecraftservices.com/minecraft/profile", frame.writer(), { .headers = headers });

        u32 numFailures = 0;
        client.awaitAll(&numFailures);
        if(numFailures > 0) {
            NBL_WARN("Minecraft profile request failed");
            return nbl::Result::IO_ERROR;
        }

        usize length = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(outputBegin);
        nbl::StringView responseStr(static_cast<char*>(outputBegin), length);

        NBL_CHECK(nbl::parseJson(responseStr, &frame, &response));
    }

    nbl::StringView uuidHex = response.asObject().getString("id");
    nbl::StringView username = response.asObject().getString("name");
    nbl::JsonArray skins = response.asObject().getArray("skins");
    nbl::StringView skin{};
    for(nbl::JsonElement& element : skins) {
        if(element.asObject().getString("state") != "ACTIVE") continue;
        skin = element.asObject().getString("url");
    }

    if(uuidHex.length() != 32 || username.length() == 0 || username.length() > 16) {
        NBL_WARN("Invalid name or UUID: name=", username, ", uuid=", uuidHex);
        return nbl::Result::FORMAT_ERROR;
    }

    nbl::StringView skinId{};
    tracker->postStatus("Downloading skin");
    if(skin.length() > 0) {
        NBL_CHECK(nbl::mc::downloadSkinHead(skinFolder, skin, &client, &skinId));
    }

    nbl::StringView skinPath{};
    NBL_CHECK(nbl::StringBuilder(&frame).append("file://").append(skinFolder).append('/').append(skinId).append(".png").buildNullTerminated(&skinPath));

    values[0].stringValue.set(username);
    values[1].stringValue.set(uuidHex);
    values[2].stringValue.set(mcAccessToken);
    values[4].uintegerValue = expiry;

    tracker->complete(context, username, skinPath, nbl::OnlineAccountFlag::ICON_NEAREST_SCALING | nbl::OnlineAccountFlag::IMAGE_ICON, values);
    return nbl::Result::SUCCESS;
}

}



nbl::Result nbl::mc::login(nbl::PluginContext* context, Span<FieldValue> values, LoginTracker* tracker, void* userData) {

    nbl::Array<char, 128> challengeB64{};

    AllocationFrame frame(Arena::threadLocal());
    StringBuilder builder(&frame);
    NBL_CHECK(tracker->beginOAuthLogin(
                "https://login.live.com/oauth20_authorize.srf", 
                MC_CLIENT_ID, 
                nbl::Array<nbl::StringView, 2> {
                    "XboxLive.signin",
                    "offline_access"
                },
                8090,
                OAuthResponseType::CODE, 
                challengeB64,
                &builder));

    StringView code{};
    NBL_CHECK(builder.buildNullTerminated(&code));

    StringView codeVerifier = StringView(challengeB64.data, 128);

    HttpClient client{};
    NBL_CHECK(HttpClient::create(
        1,1,0,1, 
        &client, 
        nbl::HttpClientFlag::CREATE_FILE_PARENTS
    ));

    StringView payload;
    NBL_CHECK(StringBuilder(&frame)
            .append("client_id=").append(MC_CLIENT_ID)
            .append("&code=").append(code)
            .append("&code_verifier=").append(codeVerifier)
            .append("&grant_type=authorization_code")
            .append("&redirect_uri=http://localhost:8090")
            .build(&payload));
    
    tracker->postStatus("Requesting Xbox Live token");

    nbl::Array headers = {
        nbl::HttpHeader { .name = "x-xbl-contract-version", .value = "1" }
    };
    void* outputBegin = frame.end();
    client.makePostRequest("https://login.live.com/oauth20_token.srf", payload, frame.writer(), { .headers = headers });

    u32 numFailures = 0;
    client.awaitAll(&numFailures);
    if(numFailures > 0) {
        NBL_WARN("Xbox Live token request failed");
        return nbl::Result::IO_ERROR;
    }

    usize length = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(outputBegin);
    StringView responseStr(static_cast<char*>(outputBegin), length);

    JsonElement element{};
    NBL_CHECK(nbl::parseJson(responseStr, &frame, &element));

    if(element.type() != JsonType::OBJECT) {
        NBL_WARN("Xbox Live sent invalid response");
        return nbl::Result::FORMAT_ERROR;
    }

    StringView token = element.asObject().getString("access_token");
    StringView refreshToken = element.asObject().getString("refresh_token");

    if(token.length() == 0 || refreshToken.length() == 0) {
        NBL_WARN("Xbox Live sent invalid response: ", element.asObject());
        return nbl::Result::FORMAT_ERROR;
    }


    values[3].stringValue.set(refreshToken);

    StringView skinFolder;
    NBL_CHECK(StringBuilder(&frame).append(getTempDirectory()).append("/nbl_skin").buildNullTerminated(&skinFolder));

    return _finishLogin(context, client, values, token, skinFolder, tracker);
}

nbl::Result nbl::mc::refresh(nbl::PluginContext* context, const OnlineAccount& account, LoginTracker* tracker, void* userData) {

    AllocationFrame frame(Arena::threadLocal());
    JsonElement element{};
    NBL_CHECK(nbl::parseJson(account.fields, &frame, &element));

    Span<FieldValue> values{};
    NBL_CHECK(nbl::decodeFieldValues(context, MC_AUTH_FIELDS, element.asObject(), &values, &frame)); 
    StringView refreshToken = values[3].stringValue.raw;
    u64 expiry = values[4].uintegerValue;
    u64 now = getCurrentTime();

    if(refreshToken.length() == 0) {
        return nbl::Result::EXPIRED;
    }
    if(now + (1000 * 60 * 60 * 12) < expiry) {
        return nbl::Result::NO_CHANGE;
    }

    StringView payload;
    NBL_CHECK(StringBuilder(&frame)
            .append("client_id=").append(MC_CLIENT_ID)
            .append("&refresh_token=").append(refreshToken)
            .append("&grant_type=refresh_token")
            .append("&redirect_uri=http://localhost:8090")
            .build(&payload));


    nbl::Array headers = {
        nbl::HttpHeader { .name = "x-xbl-contract-version", .value = "1" }
    };


    HttpClient client{};
    NBL_CHECK(HttpClient::create(
        1,1,0,1, 
        &client, 
        nbl::HttpClientFlag::CREATE_FILE_PARENTS
    ));

    void* outputBegin = frame.end();
    client.makePostRequest("https://login.live.com/oauth20_token.srf", payload, frame.writer(), { .headers = headers });

    u32 numFailures = 0;
    client.awaitAll(&numFailures);
    if(numFailures > 0) return nbl::Result::IO_ERROR;

    usize length = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(outputBegin);
    StringView responseStr(static_cast<char*>(outputBegin), length);

    JsonElement response{};
    NBL_CHECK(nbl::parseJson(responseStr, &frame, &response));

    if(response.type() != JsonType::OBJECT) {
        NBL_WARN("Xbox Live sent invalid response");
        return nbl::Result::FORMAT_ERROR;
    }

    StringView token = response.asObject().getString("access_token");
    StringView newRefreshToken = response.asObject().getString("refresh_token");

    if(token.length() == 0 || refreshToken.length() == 0) {
        NBL_WARN("Xbox Live sent invalid response: ", response.asObject());
        return nbl::Result::FORMAT_ERROR;
    }


    values[3].stringValue.set(refreshToken);

    StringView skinFolder;
    NBL_CHECK(StringBuilder(&frame).append(getTempDirectory()).append("/nbl_skin").buildNullTerminated(&skinFolder));

    return _finishLogin(context, client, values, token, skinFolder, tracker);
}

nbl::Result nbl::mc::logout(nbl::PluginContext* context, i64 accountId, void* userData) {
    return nbl::Result::SUCCESS;
}
