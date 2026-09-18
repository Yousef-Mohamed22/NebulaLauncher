#include <Plugin/PluginInitializer.hpp>
#include <Plugin/MetadataProvider.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Platform/Arena.hpp>
#include <Platform/File.hpp>
#include <Platform/Logging.hpp>
#include <Platform/StringBuilder.hpp>
#include <Plugin/PluginContext.hpp>
#include <Network/HttpClient.hpp>
#include <Codec/Json.hpp>
#include <Core/String.hpp>



namespace {

enum TriState : u32 {
    DENY = 0,
    ALLOW = 1,
    ALLOW_EXCLUSIVELY = 2
};

struct _Filters {
    u32 allowNsfw = DENY;
    u32 allowHumor = DENY;
    u32 allowFlash = DENY;
};

_Filters _filters;
char _tokenBuffer[64]{};
usize _tokenLength = 0;
nbl::HttpClient _httpClient{};

nbl::Result _login(nbl::PluginContext* context, nbl::StringView token, nbl::LoginTracker* tracker, void* userData) {

    if(token.length > 64) return nbl::Result::INVALID_ARGUMENT;
    memcpy(_tokenBuffer, token.data, token.length);
    _tokenLength = token.length;

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView apiKeyPath;

    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/sgdb/apikey.txt").buildNullTerminated(&apiKeyPath));

    nbl::FileChannel channel;
    NBL_CHECK(nbl::FileChannel::open(apiKeyPath, &channel, nbl::FileOpenFlag::WRITE | nbl::FileOpenFlag::CREATE));
    NBL_CHECK(channel.writer().writeAll(_tokenBuffer, _tokenLength));

    tracker->finishTracking(true, "");
    return nbl::Result::SUCCESS;
}
nbl::Result _logout(nbl::PluginContext* context, u32 accountId, void* userData) {

    _tokenLength = 0;
    return nbl::Result::SUCCESS;
}

bool _canProvide(nbl::PluginContext* context, const nbl::MetadataLookupInfo& info, void* userData) {
    return _tokenLength > 0 && (info.title.hasValue() || info.platformNames.contains("steam") || info.platformNames.contains("sgdb"));
}

nbl::Result _provide(nbl::PluginContext* context, const nbl::MetadataLookupInfo& info, nbl::MetadataCallback callback, void* userData) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView authHeader;
    nbl::StringBuilder(&frame).append("Bearer ").append(nbl::StringView(_tokenBuffer, _tokenLength)).buildNullTerminated(&authHeader);

    nbl::HttpHeader header = {
        .name = "Authorization",
        .value = authHeader
    };

    nbl::StringView sgdbId;
    nbl::StringView steamId;
    for(usize i = 0 ; i < info.platformIds.length ; i++) {
        if(info.platformNames[i] == "sgdb") {
            sgdbId = info.platformIds[i];
        } else if(info.platformNames[i] == "steam") {
            steamId = info.platformIds[i];
        }
    }

    if(sgdbId.empty()) {

        u64 intId;

        if(steamId.empty()) {

            usize begin = frame.index();
            if(!info.title.hasValue()) {
                return nbl::Result::INVALID_STATE;
            }

            nbl::StringView url;
            NBL_CHECK(nbl::StringBuilder(&frame)
                    .append("https://www.steamgriddb.com/api/v2/search/autocomplete/")
                    .append(nbl::UrlEncode(*info.title))
                    .buildNullTerminated(&url));

            void* responseStart = frame.end();
            NBL_CHECK(_httpClient.makeRequest(url, frame.writer(), { .headers = { &header, 1 } }));

            u32 numFailures = 0;
            _httpClient.awaitAll(&numFailures);
            if(numFailures > 0) return nbl::Result::IO_ERROR;

            usize responseBytes = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(responseStart);
            nbl::BufferStream stream(responseStart, responseBytes);

            nbl::JsonElement response{};
            NBL_CHECK(nbl::parseJson(stream, &frame, &response));

            if(response.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
            if(!response.object.getBoolean("success")) return nbl::Result::SUCCESS;

            nbl::JsonArray data = response.object.getArray("data");
            if(data.count == 0) return nbl::Result::SUCCESS;

            intId = data.getObject(0).getNumber("id").asUint();
            frame.popTo(begin);

        } else {

            usize begin = frame.index();
            nbl::StringView url;
            NBL_CHECK(nbl::StringBuilder(&frame)
                    .append("https://www.steamgriddb.com/api/v2/games/steam/")
                    .append(nbl::UrlEncode(steamId))
                    .buildNullTerminated(&url));

            void* responseStart = frame.end();
            NBL_CHECK(_httpClient.makeRequest(url, frame.writer(), { .headers = { &header, 1 } }));

            u32 numFailures = 0;
            _httpClient.awaitAll(&numFailures);
            if(numFailures > 0) return nbl::Result::IO_ERROR;


            usize responseBytes = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(responseStart);
            nbl::BufferStream stream(responseStart, responseBytes);

            nbl::JsonElement response{};
            NBL_CHECK(nbl::parseJson(stream, &frame, &response));

            if(response.type != nbl::JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
            if(!response.object.getBoolean("success")) return nbl::Result::SUCCESS;

            nbl::JsonObject data = response.object.getObject("data");
            if(data.entryCount == 0) return nbl::Result::SUCCESS;

            intId = data.getNumber("id").asUint();
            frame.popTo(begin);
        }

        NBL_CHECK(nbl::StringBuilder(&frame).append(intId).build(&sgdbId));
    }

    {
        usize begin = frame.index();

        nbl::StringView gridUrl;
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append("https://www.steamgriddb.com/api/v2/grids/game/")
                .append(nbl::UrlEncode(sgdbId))
                .append("?dimensions=600x900,512x512,1024x1024&limit=20")
                .append(_filters)
                .buildNullTerminated(&gridUrl)
        );

        void* responseStart = frame.end();
        NBL_CHECK(_httpClient.makeRequest(gridUrl, frame.writer(), { .headers = { &header, 1 } }));

        u32 numFailures = 0;
        _httpClient.awaitAll(&numFailures);
        if(numFailures > 0) return nbl::Result::IO_ERROR;

        usize responseBytes = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(responseStart);
        nbl::BufferStream stream(responseStart, responseBytes);

        nbl::JsonElement response{};
        NBL_CHECK(nbl::parseJson(stream, &frame, &response));

        if(response.type == nbl::JsonType::OBJECT && response.object.getBoolean("success")) {
            for(nbl::JsonElement e : response.object.getArray("data")) {
                if(e.type != nbl::JsonType::OBJECT) continue;

                u32 width = e.object.getNumber("width").asUint();
                u32 height = e.object.getNumber("height").asUint();
                nbl::MetadataField field = (width == height) ? nbl::MetadataField::ICON : nbl::MetadataField::POSTER;
                nbl::Result res = callback(field, { e.object.getString("url") });

                if(res != nbl::Result::SUCCESS) return res;
            }
        }
        frame.popTo(begin);
    }

    {
        usize begin = frame.index();

        nbl::StringView gridUrl;
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append("https://www.steamgriddb.com/api/v2/heroes/game/")
                .append(nbl::UrlEncode(sgdbId))
                .append("?dimensions=1920x620,3840x1240&limit=20")
                .append(_filters)
                .buildNullTerminated(&gridUrl)
        );

        void* responseStart = frame.end();
        NBL_CHECK(_httpClient.makeRequest(gridUrl, frame.writer(), { .headers = { &header, 1 } }));

        u32 numFailures = 0;
        _httpClient.awaitAll(&numFailures);
        if(numFailures > 0) return nbl::Result::IO_ERROR;

        usize responseBytes = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(responseStart);
        nbl::BufferStream stream(responseStart, responseBytes);

        nbl::JsonElement response{};
        NBL_CHECK(nbl::parseJson(stream, &frame, &response));

        if(response.type == nbl::JsonType::OBJECT && response.object.getBoolean("success")) {
            for(nbl::JsonElement e : response.object.getArray("data")) {
                if(e.type != nbl::JsonType::OBJECT) continue;

                nbl::Result res = callback(nbl::MetadataField::BANNER, { e.object.getString("url") });

                if(res != nbl::Result::SUCCESS) return res;
            }
        }
        frame.popTo(begin);
    }

    {
        usize begin = frame.index();

        nbl::StringView gridUrl;
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append("https://www.steamgriddb.com/api/v2/logos/game/")
                .append(nbl::UrlEncode(sgdbId))
                .append("?limit=20")
                .append(_filters)
                .buildNullTerminated(&gridUrl)
        );

        void* responseStart = frame.end();
        NBL_CHECK(_httpClient.makeRequest(gridUrl, frame.writer(), { .headers = { &header, 1 } }));

        u32 numFailures = 0;
        _httpClient.awaitAll(&numFailures);
        if(numFailures > 0) return nbl::Result::IO_ERROR;

        usize responseBytes = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(responseStart);
        nbl::BufferStream stream(responseStart, responseBytes);

        nbl::JsonElement response{};
        NBL_CHECK(nbl::parseJson(stream, &frame, &response));

        if(response.type == nbl::JsonType::OBJECT && response.object.getBoolean("success")) {
            for(nbl::JsonElement e : response.object.getArray("data")) {
                if(e.type != nbl::JsonType::OBJECT) continue;
                u64 pixels = e.object.getNumber("width").asUint() * e.object.getNumber("height").asUint();
                if(pixels < (32*32) || pixels > (4096*4096)) continue;

                nbl::Result res = callback(nbl::MetadataField::LOGO, { e.object.getString("url") });

                if(res != nbl::Result::SUCCESS) return res;
            }
        }
        frame.popTo(begin);
    }

    {
        usize begin = frame.index();

        nbl::StringView gridUrl;
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append("https://www.steamgriddb.com/api/v2/icons/game/")
                .append(nbl::UrlEncode(sgdbId))
                .append("?limit=20")
                .append(_filters)
                .buildNullTerminated(&gridUrl)
        );

        void* responseStart = frame.end();
        NBL_CHECK(_httpClient.makeRequest(gridUrl, frame.writer(), { .headers = { &header, 1 } }));

        u32 numFailures = 0;
        _httpClient.awaitAll(&numFailures);
        if(numFailures > 0) return nbl::Result::IO_ERROR;

        usize responseBytes = reinterpret_cast<usize>(frame.end()) - reinterpret_cast<usize>(responseStart);
        nbl::BufferStream stream(responseStart, responseBytes);

        nbl::JsonElement response{};
        NBL_CHECK(nbl::parseJson(stream, &frame, &response));

        if(response.type == nbl::JsonType::OBJECT && response.object.getBoolean("success")) {
            for(nbl::JsonElement e : response.object.getArray("data")) {
                if(e.type != nbl::JsonType::OBJECT) continue;
                u64 pixels = e.object.getNumber("width").asUint() * e.object.getNumber("height").asUint();
                if(pixels < (32*32) || pixels > (4096*4096)) continue;

                nbl::Result res = callback(nbl::MetadataField::ICON, { e.object.getString("url") });

                if(res != nbl::Result::SUCCESS) return res;
            }
        }
        frame.popTo(begin);
    }


    return nbl::Result::SUCCESS;
}

nbl::Result _getAccount(nbl::PluginContext* context, u32 accountId, nbl::AccountInfo* info, void* userData) {

    if(accountId > 0 || _tokenLength == 0) return nbl::Result::NOT_FOUND;
    info->id = 0;
    info->icon = "";
    info->userName = "API Key";

    return nbl::Result::SUCCESS;
}

nbl::Result _enumerateAccounts(nbl::PluginContext* context, nbl::AccountCallback callback, void* userData) {
    if(_tokenLength > 0) {
        callback({
            .userName = "API Key",
            .icon = "",
            .id = 0
        });
    }
    return nbl::Result::SUCCESS;
}

nbl::AccountProvider _provider {
    .id = "sgdb",
    .name = "SteamGridDB",
    .type = nbl::AccountProvider::Type::TOKEN,
    .maxAccounts = 1,
    .tokenData = {
        .pfnLogin = _login,
        .pfnLogout = _logout,
        .userData = nullptr,
        .tokenText = "API Key"
    },
    .accountInfo = {
        .pfnGet = _getAccount,
        .pfnEnumerate = _enumerateAccounts
    }
};

nbl::MetadataProvider _metaProvider {
    .id = "sgdb",
    .name = "SteamGridDB",
    .pfnCanProvide = _canProvide,
    .pfnProvide = _provide,
    .userData = nullptr
};

u32 _tristateIndex(nbl::StringView name, void* userData) {
    if(name == "true") return ALLOW_EXCLUSIVELY;
    if(name == "any") return ALLOW;
    return DENY;
}

u32 _tristateCount(void* userData) { return 3; }

nbl::StringView _tristateName(u32 index, void* userData) {
    switch(index) {
        default:
        case DENY: return "false";
        case ALLOW: return "any";
        case ALLOW_EXCLUSIVELY: return "true";
    }
}nbl::StringView _tristateDisplayName(u32 index, void* userData) {
    switch(index) {
        default:
        case DENY: return "Deny";
        case ALLOW: return "Allow";
        case ALLOW_EXCLUSIVELY: return "Allow Exclusively";
    }
}

nbl::Array _pluginFields {
    nbl::Field {
        .type = nbl::FieldType::ENUM,
        .key = "nsfw",
        .name = "NSFW Filter",
        .enumData = {
            .pfnIndex = _tristateIndex,
            .pfnName = _tristateName,
            .pfnDisplayName = _tristateDisplayName,
            .pfnCount = _tristateCount
        },
        .defaultValue = { .enumValue = DENY }
    },
        nbl::Field {
        .type = nbl::FieldType::ENUM,
        .key = "humor",
        .name = "Humor Filter",
        .enumData = {
            .pfnIndex = _tristateIndex,
            .pfnName = _tristateName,
            .pfnDisplayName = _tristateDisplayName,
            .pfnCount = _tristateCount
        },
        .defaultValue = { .enumValue = DENY }
    },
        nbl::Field {
        .type = nbl::FieldType::ENUM,
        .key = "epilepsy",
        .name = "Epilepsy Filter",
        .enumData = {
            .pfnIndex = _tristateIndex,
            .pfnName = _tristateName,
            .pfnDisplayName = _tristateDisplayName,
            .pfnCount = _tristateCount,
        },
        .defaultValue = { .enumValue = DENY }
    }
};

}

extern "C" i32 initPlugin(nbl::PluginInitContext* context) {

    nbl::log(nbl::LogLevel::INFO, "Initializing SteamGridDB plugin...");
    NBL_CHECK(context->registerObject(&_provider));
    NBL_CHECK(context->registerObject(&_metaProvider));
    
    context->setFields(_pluginFields, [](nbl::Span<nbl::Field> fields, nbl::Span<nbl::FieldValue> fieldValues, void* userData) {
        _filters.allowNsfw = fieldValues[0].enumValue;
        _filters.allowHumor = fieldValues[1].enumValue;
        _filters.allowFlash = fieldValues[2].enumValue;
    }, nullptr);

    {
        nbl::ArenaFrame frame(nbl::Arena::threadLocal());
        nbl::Span<nbl::FieldValue> values{};
        NBL_CHECK(context->getStoredFieldValues(_pluginFields, &values, &frame));
        _filters.allowNsfw = values[0].enumValue;
        _filters.allowHumor = values[1].enumValue;
        _filters.allowFlash = values[2].enumValue;
    }

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView pluginFolder;
    nbl::StringView apiKeyPath;

    NBL_CHECK(nbl::HttpClient::create(1, 1, 0, 0, &_httpClient));

    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/sgdb").buildNullTerminated(&pluginFolder));
    NBL_CHECK(nbl::StringBuilder(&frame).append(pluginFolder).append("/apikey.txt").buildNullTerminated(&apiKeyPath));
    NBL_CHECK(nbl::makeDirectories(pluginFolder));

    nbl::FileChannel channel;
    nbl::Result res = nbl::FileChannel::open(apiKeyPath, &channel, nbl::FileOpenFlag::READ);

    if(res.error() && res != nbl::Result::NOT_FOUND) {
        return res;
    }

    if(res == nbl::Result::SUCCESS) {
        usize justRead = 0;
        while((res = channel.reader().read(_tokenBuffer + _tokenLength, 64 - _tokenLength, &justRead)).success()) {
            _tokenLength += justRead;
        }
        if(res.error() && res != nbl::Result::END_OF_STREAM) {
            return res;
        }
    }


    return 0;
}

namespace nbl {
Result toString(const _Filters& filters, Writer writer);
}

nbl::Result nbl::toString(const _Filters& filters, Writer writer) {
    if(filters.allowNsfw != DENY) {
        NBL_CHECK(writer.writeString("&nsfw="));
        NBL_CHECK(writer.writeString(_tristateName(filters.allowNsfw, nullptr)));
    }
    if(filters.allowHumor != DENY) {
        NBL_CHECK(writer.writeString("&humor="));
        NBL_CHECK(writer.writeString(_tristateName(filters.allowHumor, nullptr)));
    }
    if(filters.allowFlash != DENY) {
        NBL_CHECK(writer.writeString("&epilepsy="));
        NBL_CHECK(writer.writeString(_tristateName(filters.allowFlash, nullptr)));
    }
    return nbl::Result::SUCCESS;
}
