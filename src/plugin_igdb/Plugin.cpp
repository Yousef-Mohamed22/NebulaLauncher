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
#include <utility>
#include <Plugin/Field.hpp>
#include "Data/Model.hpp"



namespace {

char _clientIdBuffer[256];
char _clientSecretBuffer[256];
char _accessTokenBuffer[256];


enum _ExternalGameSource : u32 {
    EXT_STEAM = 1,
    EXT_GIANTBOMB = 3,
    EXT_GOG = 5,
    EXT_YOUTUBE = 10,
    EXT_MICROSOFT = 11,
    EXT_APPLE = 13,
    EXT_TWITCH = 14,
    EXT_ANDROID = 15,
    EXT_AMAZON = 20,
    EXT_AMAZON_LUNA = 22,
    EXT_AMAZON_ADG = 23,
    EXT_EGS = 26,
    EXT_OCULUS = 28,
    EXT_UTOMIK = 29,
    EXT_ITCHIO = 30,
    EXT_XBOX = 31,
    EXT_KARTRIDGE = 32,
    EXT_PS = 36,
    EXT_FOCUS = 37,
    EXT_XBOX_GPU_CLOUD = 54,
    EXT_GAMEJOLT = 55
};


nbl::StringView _externalSourceName(u32 index) {
    switch(index) {
        case EXT_STEAM: return "steam";
    	case EXT_GIANTBOMB: return "giantbomb";
    	case EXT_GOG: return "gog";
    	case EXT_YOUTUBE: return "youtube";
    	case EXT_MICROSOFT: return "microsoft_store";
    	case EXT_APPLE: return "app_store";
    	case EXT_TWITCH: return "twitch";
    	case EXT_ANDROID: return "play_store";
    	case EXT_AMAZON: return "amazon_appstore";
    	case EXT_AMAZON_LUNA: return "amazon_luna";
    	case EXT_AMAZON_ADG: return "amazon_digital_games";
    	case EXT_EGS: return "epic_games";
    	case EXT_OCULUS: return "oculus";
    	case EXT_UTOMIK: return "utomik";
    	case EXT_ITCHIO: return "itchio";
    	case EXT_XBOX: return "xbox";
    	case EXT_KARTRIDGE: return "kartridge";
    	case EXT_PS: return "playstation";
    	case EXT_FOCUS: return "focus_entertainment";
    	case EXT_XBOX_GPU_CLOUD: return "xbox_gamepass_ultimate_cloud";
    	case EXT_GAMEJOLT: return "gamejolt";
    }
    return "";
};


enum _TagMode : u32 {
    TAG_MODE_ALL = 0,
    TAG_MODE_NONE = 1,
    TAG_MODE_MAPPED = 2
};

u32 _tagModeIndex(nbl::StringView name, void* userData) {
    if(name == "none") return TAG_MODE_NONE;
    if(name == "mapped") return TAG_MODE_MAPPED;
    return TAG_MODE_ALL;
}

u32 _tagModeCount(void* userData) { return 3; }

nbl::StringView _tagModeName(u32 index, void* userData) {
    switch(index) {
        default:
        case TAG_MODE_ALL: return "all";
        case TAG_MODE_NONE: return "none";
        case TAG_MODE_MAPPED: return "mapped";
    }
}
nbl::StringView _tagModeDisplayName(u32 index, void* userData) {
    switch(index) {
        default:
        case TAG_MODE_ALL: return "All Tags";
        case TAG_MODE_NONE: return "No Tags";
        case TAG_MODE_MAPPED: return "Mapped Tags";
    }
}


struct _TagMapping {
    u64 tagHash;
    nbl::StringView igdbTagName;
    nbl::StringView mappedTagName;
};
nbl::Arena _configArena{};
nbl::Span<_TagMapping> _mappings{};
_TagMode _tagMode = TAG_MODE_ALL;


i32 _partitionMappings(_TagMapping* mappings, i32 low, i32 high) {

    u64 pivot = mappings[high].tagHash;
    i32 i = low - 1;

    for(i32 j = low; j <= high - 1; j++) {
        if(mappings[j].tagHash < pivot) {
            i++;
            std::swap(mappings[i], mappings[j]);
        }
    }

    std::swap(mappings[i + 1], mappings[high]);
    return i + 1;
}

void _quicksortMappings(_TagMapping* mappings, i32 low, i32 high) {

    if(low >= high) return;

    i32 pivotIndex = _partitionMappings(mappings, low, high);

    _quicksortMappings(mappings, low, pivotIndex - 1);
    _quicksortMappings(mappings, pivotIndex + 1, high);

}

nbl::Optional<nbl::StringView> _findMapping(nbl::StringView tag) {

    if(_mappings.length == 0) return {};
    if(_mappings.length == 1) {
        if(_mappings[0].igdbTagName == tag) {
            return _mappings[0].mappedTagName;
        }
        return {};
    }

    u64 hash = tag.hash();

    i32 low = 0;
    i32 high = _mappings.length - 1;

    while(low <= high) {

        i32 index = low + (high - low) / 2;
        _TagMapping& midpointTag = _mappings[index];

        if(midpointTag.tagHash == hash) {
            if(midpointTag.igdbTagName == tag) {
                return midpointTag.mappedTagName;
            } else {
                // Hash collision. Very unlikely but we account for it.
                low = index;
                high = index + 1;
                while(low > 0) {
                    low--;
                    if(_mappings[low].tagHash != hash) break;
                    if(_mappings[low].igdbTagName == tag) return _mappings[low].mappedTagName;
                }
                while(high < _mappings.length - 1) {
                    if(_mappings[high].tagHash != hash) break;
                    if(_mappings[high].igdbTagName == tag) return _mappings[high].mappedTagName;
                    high++;
                }
                return {};
            }
        }

        if(midpointTag.tagHash < hash) {
            low = index + 1;
        } else {
            high = index - 1;
        }
    }
    return {};
};


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
    nbl::StringView fields = "fields aggregated_rating,artworks.image_id,artworks.image_type,cover.image_id,cover.image_type,first_release_date,name,summary,external_games.uid,external_games.external_game_source";

    nbl::StringBuilder builder(&frame);
    builder.append(fields);
    if(_tagMode != TAG_MODE_NONE) {
        builder.append(",themes.slug,genres.slug,player_perspectives.slug");
    }
    builder.append("; limit 20;");

    if(info.meaning == nbl::OnlineGameLookupInfo::TITLE) {
        void* start = frame.end();
        usize startIndex = frame.index();
        NBL_CHECK(nbl::writeJson(frame.writer(), info.param)); // Write as a JSON string to add quotes escape control characters
        nbl::StringView escapedTitle(static_cast<char*>(start), frame.index() - startIndex);

        builder
            .append(" search ")
            .append(escapedTitle)
            .append(";");

    } else {

        nbl::JsonNumber id;
        NBL_CHECK(nbl::parseJsonNumber(nbl::StringReader(info.param), &id));

        if(id.type != nbl::JsonNumberType::UNSIGNED) return nbl::Result::FORMAT_ERROR;

        builder
            .append(" where id = ")
            .append(id.asUint())
            .append(";");
    }

    nbl::StringView body;
    NBL_CHECK(builder.buildNullTerminated(&body));

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
            if(meta.posterUrl.length == 0 && (imageType == 8 || imageType == 10)) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.posterUrl));
            }
            if(meta.iconUrl.length == 0 && (imageType == 11 || imageType == 13)) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.iconUrl));
            }
            if(meta.logoUrl.length == 0 && (imageType == 5 || imageType == 6 || imageType == 7)) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.logoUrl));
            }
            if(meta.bannerUrl.length == 0 && (imageType == 1 || imageType == 2 || imageType == 3)) {
                NBL_CHECK(nbl::StringBuilder(&subFrame).append("https://images.igdb.com/igdb/image/upload/t_original/").append(element.object.getString("image_id")).append(".png").build(&meta.bannerUrl));
                if(imageType == 3) { // Key art with logo
                    meta.logoPosition = nbl::LogoPosition::HIDDEN;
                }
            }
        }

        nbl::StringView idStr;
        NBL_CHECK(nbl::StringBuilder(&subFrame).append(id).build(&idStr));

        nbl::JsonArray arr = obj.getArray("external_games");

        subFrame.allocate(&meta.platformNames.data, arr.count + 1);
        subFrame.allocate(&meta.platformIds.data, arr.count + 1);

        meta.platformNames[meta.platformNames.length++] = "igdb";
        meta.platformIds[meta.platformIds.length++] = idStr;

        if(arr.count > 0) {

            for(nbl::JsonElement element : arr) {
                if(element.type != nbl::JsonType::OBJECT) continue;
                nbl::JsonObject& obj = element.object;

                nbl::StringView uid = obj.getString("uid");
                u64 sourceId = obj.getNumber("external_game_source").asUint();

                nbl::StringView platformName = _externalSourceName(sourceId);
                if(uid.empty() || platformName.empty()) continue;

                meta.platformNames[meta.platformNames.length++] = platformName;
                meta.platformIds[meta.platformIds.length++] = uid;
            }
        }

        if(_tagMode != TAG_MODE_NONE) {

            nbl::JsonArray themes = obj.getArray("themes");
            nbl::JsonArray genres = obj.getArray("genres");
            nbl::JsonArray perspectives = obj.getArray("player_perspectives");

            usize maxTagCount = themes.count + genres.count + perspectives.count;
            if(maxTagCount > 0) {

                nbl::Array allTags = { themes, genres, perspectives };
                NBL_CHECK(subFrame.allocate(&meta.tags.data, maxTagCount));

                for(nbl::JsonArray a : allTags) {
                    for(nbl::JsonElement e : a) {
                        if(e.type != nbl::JsonType::OBJECT) continue;
                        nbl::StringView slug = e.object.getString("slug");

                        nbl::StringView tag{};
                        NBL_CHECK(nbl::StringBuilder(&subFrame).append(slug)
                                .replace('-',"_")
                                .buildNullTerminated(&tag));

                        nbl::Optional<nbl::StringView> mapping = _findMapping(tag);
                        if(mapping.hasValue()) {
                            nbl::log(nbl::LogLevel::DEBUG, "Found mapping: ", tag, " -> ", mapping);
                            tag = *mapping;
                        } else if(_tagMode == TAG_MODE_MAPPED) continue;

                        if(tag.length > 0) {
                            meta.tags.length++;
                            meta.tags[meta.tags.length - 1] = tag;
                        }
                    }
                }
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


nbl::Array _mappingFields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "id",
        .name = "IGDB Tag Name"
    },
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "value",
        .name = "Nebula Tag Name"
    }
};

nbl::Field _listField = {
    .type = nbl::FieldType::STRUCT,
    .key = "",
    .name = "",
    .structData = {
        .fields = _mappingFields.span(),
        .horizontal = true
    },
    .defaultValue = {}
};


nbl::Array _pluginFields = {
    nbl::Field { 
        .type = nbl::FieldType::ENUM,
        .key = "tag_mode",
        .name = "Apply Tags",
        .enumData = {
            .pfnIndex = _tagModeIndex,
            .pfnName = _tagModeName,
            .pfnDisplayName = _tagModeDisplayName,
            .pfnCount = _tagModeCount,
            .userData = nullptr
        },
        .defaultValue = {
            .enumValue = TAG_MODE_ALL
        }
    },
    nbl::Field {
        .type = nbl::FieldType::LIST,
        .key = "tag_mappings",
        .name = "Tag Mappings",
        .listData = {
            .templateField = &_listField,
            .min = 0,
            .max = 999,
        }
    }
};


nbl::Result _loadConfig(nbl::Span<nbl::FieldValue> values) {

    _configArena.popTo(0);

    _TagMode tagMode = static_cast<_TagMode>(values[0].enumValue);
    if(tagMode != TAG_MODE_NONE && values[1].compositeValue.length > 0) {

        _TagMapping* mappings;
        NBL_CHECK(_configArena.allocate(&mappings, values[1].compositeValue.length));

        usize index = 0;
        for(nbl::FieldValue mapping : values[1].compositeValue) {

            void* igdb = _configArena.end();
            usize igdbIndex = _configArena.index();
            NBL_CHECK(_configArena.writer().writeString(mapping.compositeValue[0].stringValue.resolved));
            usize length = _configArena.index() - igdbIndex;

            void* mapped = _configArena.end();
            usize mappedIndex = _configArena.index();
            NBL_CHECK(_configArena.writer().writeString(mapping.compositeValue[1].stringValue.resolved));
            usize mappedLength = _configArena.index() - mappedIndex;

            mappings[index++] = _TagMapping {
                .tagHash = nbl::StringView(static_cast<char*>(igdb), length).hash(),
                .igdbTagName = nbl::StringView(static_cast<char*>(igdb), length),
                .mappedTagName = nbl::StringView(static_cast<char*>(mapped), mappedLength)
            };
        }

        _mappings = nbl::Span(mappings, values[1].compositeValue.length);
        _quicksortMappings(_mappings.data, 0, _mappings.length - 1);
    }

    return nbl::Result::SUCCESS;
}

}

extern "C" i32 initPlugin(nbl::PluginInitContext* context) {

    nbl::log(nbl::LogLevel::INFO, "Initializing IGDB plugin...");
    context->registerAccountProvider(&_provider);
    context->registerGameProvider(&_gameProvider);

    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 4, &_configArena));

    context->setFields(_pluginFields, [](nbl::Span<nbl::Field> fields, nbl::Span<nbl::FieldValue> values, void* userData) {
        _loadConfig(values);
    }, nullptr);


    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    NBL_CHECK(nbl::HttpClient::create(4, 4, 0, 0, &_client));

    nbl::StringView folderPath{};
    NBL_CHECK(nbl::StringBuilder(&frame).append(context->pluginDataPath()).append("/igdb").buildNullTerminated(&folderPath));
    NBL_CHECK(nbl::makeDirectories(folderPath));

    nbl::StringView path{};
    NBL_CHECK(nbl::StringBuilder(&frame).append(folderPath).append("/auth.json").buildNullTerminated(&path));

    nbl::Result res = loadAuth(path);
    if(res.error() && res != nbl::Result::NOT_FOUND) {
        nbl::log(nbl::LogLevel::WARN, "An error occurred while loading IGDB authentication details!");
    }

    nbl::Span<nbl::FieldValue> values{};
    NBL_CHECK(context->getStoredFieldValues(_pluginFields, &values, &frame));

    NBL_CHECK(_loadConfig(values));

    return 0;
}
