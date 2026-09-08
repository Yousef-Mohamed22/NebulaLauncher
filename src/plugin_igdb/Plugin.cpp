#include <Plugin/PluginInitializer.hpp>
#include <Plugin/GameProvider.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Platform/Arena.hpp>
#include <Platform/File.hpp>
#include <Platform/Logging.hpp>
#include <Platform/StringBuilder.hpp>
#include <Plugin/PluginContext.hpp>
#include <Network/HttpClient.hpp>
#include <Codec/Json.hpp>
#include <Core/String.hpp>
#include <Core/Ring.hpp>
#include <Core/Stream.hpp>
#include <Platform/Time.hpp>



namespace {

char _clientIdBuffer[256];
char _clientSecretBuffer[256];
char _accessTokenBuffer[256];

nbl::Ring<u64, 4> _requestTimes{};
nbl::HttpClient _client{};

nbl::StringView _clientId;
nbl::StringView _clientSecret;
nbl::StringView _accessToken;

nbl::Result _doLogin(nbl::StringView clientId, nbl::StringView clientSecret, nbl::StringView authFile) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView url;

    NBL_CHECK(nbl::StringBuilder(&frame)
            .append("https://id.twitch.tv/oauth2/token?client_id=")
            .append(nbl::UrlEncode(clientId))
            .append("&client_secret=")
            .append(nbl::UrlEncode(clientSecret))
            .append("&grant_type=client_credentials")
            .buildNullTerminated(&url));

    void* start = frame.end();
    NBL_CHECK(_client.makePostRequest(url, "", frame.writer()));

    u32 failures = 0;
    _client.awaitAll(&failures);
    if(failures > 0) {
        return nbl::Result::IO_ERROR;
    }

    nbl::BufferStream stream(start, reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(start));
    nbl::JsonElement response;

    NBL_CHECK(nbl::parseJson(stream, &frame, &response));
    if(response.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    nbl::StringView token = response.object.getString("access_token");
    if(token.length > 256) return nbl::Result::FORMAT_ERROR;

    u64 expiresIn = response.object.getNumber("expires_in").asUint();

    memcpy(_accessTokenBuffer, token.data, token.length);
    memcpy(_clientSecretBuffer, clientSecret.data, clientSecret.length);
    memcpy(_clientIdBuffer, clientId.data, clientId.length);

    _accessToken = nbl::StringView(_accessTokenBuffer, token.length);
    _clientSecret = nbl::StringView(_clientSecretBuffer, clientSecret.length);
    _clientId = nbl::StringView(_clientIdBuffer, clientId.length);


    nbl::FileChannel channel;
    NBL_CHECK(nbl::FileChannel::open(authFile, &channel, nbl::FileOpenFlag::CREATE | nbl::FileOpenFlag::WRITE));

    u64 now = nbl::getCurrentTime();

    nbl::JsonWriter(channel.writer()).writeObject([&](auto obj) {
        obj.add("client_id", _clientId)
            .add("client_secret", _clientSecret)
            .add("access_token", _accessToken)
            .add("expires_at", now + expiresIn)
            .add("total_duration", expiresIn);
    });

    return nbl::Result::SUCCESS;


}

nbl::Result _login(nbl::PluginContext* context, nbl::StringView clientId, nbl::StringView clientSecret, nbl::LoginTracker* tracker, void* userData) {

    if(clientId.length > 256 || clientSecret.length > 256) return nbl::Result::INVALID_ARGUMENT;

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringView path;
    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/igdb/auth.json").buildNullTerminated(&path));
    NBL_CHECK(_doLogin(clientId, clientSecret, path));

    tracker->finishTracking(true, {});

    return nbl::Result::SUCCESS;
}
nbl::Result _logout(nbl::PluginContext* context, u32 accountId, void* userData) {

    _accessToken.length = 0;
    _clientId.length = 0;
    _clientSecret.length = 0;

    memset(_accessTokenBuffer, 0, 256);
    memset(_clientSecretBuffer, 0, 256);
    memset(_clientIdBuffer, 0, 256);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringView path;
    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/igdb/auth.json").buildNullTerminated(&path));

    nbl::FileChannel channel;
    NBL_CHECK(nbl::FileChannel::open(path, &channel, nbl::FileOpenFlag::CREATE | nbl::FileOpenFlag::WRITE));
    NBL_CHECK(channel.writer().writeString("{}"));

    return nbl::Result::SUCCESS;
}

bool _canProvide(nbl::PluginContext* context, const nbl::OnlineGameLookupInfo& info, void* userData) {

    return _clientId.length > 0 && _accessToken.length > 0;
}

nbl::Result _provide(nbl::PluginContext* context, const nbl::OnlineGameLookupInfo& info, nbl::OnlineGameCallback callback, void* userData) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView body;
    nbl::StringView fields = "fields aggregated_rating,artworks.image_id,artworks.image_type,cover.image_id,cover.image_type,first_release_date,name,summary; limit 20;";

    if(info.meaning == nbl::OnlineGameLookupInfo::TITLE) {
        NBL_CHECK(nbl::writeJson(frame.writer(), info.param)); // Write as a JSON string to add quotes escape control characters
        nbl::StringView escapedTitle(static_cast<char*>(frame.begin()), frame.index());

        NBL_CHECK(nbl::StringBuilder(&frame)
                .append(fields)
                .append(" search ")
                .append(escapedTitle)
                .append(";")
                .buildNullTerminated(&body));

    } else {

        nbl::JsonNumber id;
        NBL_CHECK(nbl::parseJsonNumber(nbl::StringReader(info.param), &id));

        if(id.type != nbl::JsonNumberType::UNSIGNED) return nbl::Result::FORMAT_ERROR;

        NBL_CHECK(nbl::StringBuilder(&frame)
                .append(fields)
                .append(" where id = ")
                .append(id.asUint())
                .append(";")
                .buildNullTerminated(&body));
    }

    nbl::StringView auth;
    NBL_CHECK(nbl::StringBuilder(&frame).append("Bearer ").append(_accessToken).buildNullTerminated(&auth));

    nbl::Array<nbl::HttpHeader, 3> headers = {
        nbl::HttpHeader{ .name = "Client-ID", .value = _clientId },
        nbl::HttpHeader{ .name = "Authorization", .value = auth },
        nbl::HttpHeader{ .name = "Accept", .value = "application/json" }
    };

    void* start = frame.end();
    NBL_CHECK(_client.makePostRequest("https://api.igdb.com/v4/games", body, frame.writer(), { .headers = headers } ));

    u32 failures = 0;
    _client.awaitAll(&failures);
    if(failures > 0) {
        return nbl::Result::IO_ERROR;
    }

    nbl::BufferStream stream(start, reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(start));
    nbl::JsonElement response;

    NBL_CHECK(nbl::parseJson(stream, &frame, &response));
    if(response.type != nbl::JsonType::ARRAY) return nbl::Result::FORMAT_ERROR;

    for(nbl::JsonElement ele : response.array) {

        nbl::ArenaFrame subFrame(nbl::Arena::threadLocal());

        if(ele.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
        nbl::JsonObject obj = ele.object;

        nbl::OnlineGameMeta meta{};

        u64 id = obj.getNumber("id").asUint();
        meta.rating = obj.getNumber("aggregated_rating").asFloat();
        meta.releaseDate = obj.getNumber("first_release_date").asUint() * 1000;

        meta.title = obj.getString("name");
        meta.description = obj.getString("summary");


        nbl::JsonObject cover = obj.getObject("cover");
        u64 coverImageType = cover.getNumber("image_type").asUint();
        if(coverImageType == 8) {
            NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(cover.getString("image_id")).append(".png").build(&meta.posterUrl));
        }

        for(nbl::JsonElement element : obj.getArray("artworks")) {
            if(element.type != nbl::JsonType::OBJECT) continue;

            u64 imageType = element.object.getNumber("image_type").asUint();
            if(meta.posterUrl.length == 0 && imageType == 8) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.posterUrl));
            }
            if(meta.iconUrl.length == 0 && (imageType == 11 || imageType == 13)) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.iconUrl));
            }
            if(meta.logoUrl.length == 0 && (imageType == 5 || imageType == 6)) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.logoUrl));
            }
            if(meta.bannerUrl.length == 0 && imageType == 2) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.bannerUrl));
            }
        }

        NBL_CHECK(callback(meta));
    }

    return nbl::Result::SUCCESS;
}

nbl::Result _getAccount(nbl::PluginContext* context, u32 accountId, nbl::AccountInfo* info, void* userData) {

    if(accountId > 0 || _clientId.length == 0 || _accessToken.length == 0) return nbl::Result::NOT_FOUND;
    info->id = 0;
    info->icon = "";
    info->userName = "Twitch Developer Application";

    return nbl::Result::SUCCESS;
}

nbl::Result _enumerateAccounts(nbl::PluginContext* context, nbl::AccountCallback callback, void* userData) {
    if(_clientId.length > 0 && _accessToken.length > 0) {
        callback({
            .userName = "Twitch Developer Application",
            .icon = "",
            .id = 0
        });
    }
    return nbl::Result::SUCCESS;
}

nbl::AccountProvider _provider {
    .id = "igdb",
    .name = "IGDB",
    .type = nbl::AccountProvider::Type::BASIC,
    .maxAccounts = 1,
    .basicData = {
        .pfnLogin = _login,
        .pfnLogout = _logout,
        .userData = nullptr,
        .usernameText = "Client ID",
        .passwordText = "Client Secret"
    },
    .accountInfo = {
        .pfnGet = _getAccount,
        .pfnEnumerate = _enumerateAccounts
    }
};

nbl::GameProvider _gameProvider {
    .id = "igdb",
    .name = "IGDB",
    .pfnCanProvide = _canProvide,
    .pfnProvide = _provide,
    .userData = nullptr
};


nbl::Result loadAuth(nbl::StringView file) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::JsonElement element;
    {
        nbl::FileChannel channel{};
        NBL_CHECK(nbl::FileChannel::open(file, &channel, nbl::FileOpenFlag::READ));
        NBL_CHECK(nbl::parseJson(channel.reader(), &frame, &element));
    }

    if(element.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
    if(element.object.entryCount == 0) return nbl::Result::SUCCESS; // Special case

    nbl::StringView clientId = element.object.getString("client_id");
    nbl::StringView clientSecret = element.object.getString("client_secret");
    nbl::StringView accessToken = element.object.getString("access_token");
    u64 expiry = element.object.getNumber("expires_at").asUint();
    u64 totalDuration = element.object.getNumber("total_duration").asUint();

    if(clientId.empty() || clientId.length > 256 || clientSecret.empty() || clientSecret.length > 256) {
        return nbl::Result::FORMAT_ERROR;
    }

    u64 now = nbl::getCurrentTime();

    u64 remaining = expiry < now ? 0 : expiry - now;
    if(totalDuration == 0 || static_cast<f64>(remaining) / static_cast<f64>(totalDuration) < 0.1f) {  

        return _doLogin(clientId, clientSecret, file);

    } else {
        if(accessToken.empty() || accessToken.length > 256) {
            return nbl::Result::FORMAT_ERROR;
        }

        memcpy(_clientIdBuffer, clientId.data, clientId.length);
        memcpy(_clientSecretBuffer, clientSecret.data, clientSecret.length);
        memcpy(_accessTokenBuffer, accessToken.data, accessToken.length);

        _accessToken = nbl::StringView(_accessTokenBuffer, accessToken.length);
        _clientSecret = nbl::StringView(_clientSecretBuffer, clientSecret.length);
        _clientId = nbl::StringView(_clientIdBuffer, clientId.length);

        return nbl::Result::SUCCESS;
    }
}

}

extern "C" i32 initPlugin(nbl::PluginInitContext* context) {

    nbl::log(nbl::LogLevel::INFO, "Initializing IGDB plugin...");
    context->registerAccountProvider(&_provider);
    context->registerGameProvider(&_gameProvider);
   
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    NBL_CHECK(nbl::HttpClient::create(4, 4, 0, 0, &_client));

    nbl::StringView folderPath{};
    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/igdb").buildNullTerminated(&folderPath));
    NBL_CHECK(nbl::makeDirectories(folderPath));

    nbl::StringView path{};
    NBL_CHECK(nbl::StringBuilder(&frame).append(folderPath).append("/auth.json").buildNullTerminated(&path));

    nbl::Result res = loadAuth(path);
    if(res.error() && res != nbl::Result::NOT_FOUND) {
        nbl::log(nbl::LogLevel::WARN, "An error occurred while loading authentication details!");
    }

    return 0;
}
