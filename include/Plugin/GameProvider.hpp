#pragma once

#include <Core/String.hpp>
#include <Plugin/PluginContext.hpp>

#include <functional>
#include <Data/Model.hpp>

namespace nbl {

struct OnlineGameLookupInfo {
    enum ParamMeaning {
        TITLE,
        ID
    };
    StringView param;
    ParamMeaning meaning;
};

struct OnlineGameMeta {
    StringView title;
    StringView description;
    StringView posterUrl;
    StringView iconUrl;
    StringView bannerUrl;
    StringView logoUrl;
    LogoPosition logoPosition;
    u64 releaseDate;
    f64 rating;
    Span<StringView> tags;
    Span<StringView> platformNames;
    Span<StringView> platformIds;
};


typedef std::function_ref<Result(const OnlineGameMeta&)> OnlineGameCallback;

struct GameProvider {

    StringView id;
    StringView name;
    bool (*pfnCanProvide)(PluginContext* context, const OnlineGameLookupInfo& info, void* userData);
    Result (*pfnProvide)(PluginContext* context, const OnlineGameLookupInfo& info, OnlineGameCallback callback, void* userData);
    void* userData;

    bool canProvide(PluginContext* context, OnlineGameLookupInfo info) const {
        if(!pfnCanProvide) return nbl::Result::PLUGIN_ERROR;
        return pfnCanProvide(context, info, userData);
    }

    Result provide(PluginContext* context, OnlineGameLookupInfo info, OnlineGameCallback callback) const {
        if(!pfnProvide) return nbl::Result::PLUGIN_ERROR;
        return pfnProvide(context, info, callback, userData);
    }
};

}
