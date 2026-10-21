#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Platform/Arena.hpp>
#include <Codec/Json.hpp>
#include <Network/HttpClient.hpp>
#include <Plugin/ProcessTracker.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/PluginInitContext.hpp>
#include <Data/OnlineAccountModel.hpp>

#include "ModLoader.hpp"

namespace nbl::mc {


class MinecraftLauncher {
public:


    Result launch(PluginContext* context, StringView version, const OnlineAccount& account, ModLoader modLoader, StringView workingDir, i64 installationId, ProcessTracker* tracker);

    inline StringView rootFolder() const {
        return _mRootFolder;
    }

    void suggestVersions(PluginContext* context, StringView current, std::function_ref<void(nbl::StringView)> suggestionConsumer);

    static Result init(StringView rootFolder, MinecraftLauncher* launcher);

private:

    static void _manifestDownloadComplete(void* userData, nbl::Result res, i32 responseCode);

    StringView _mRootFolder;
    HttpClient _mHttpClient;
    Arena _mDataArena;
    Optional<JsonObject> _mVersionManifest;


};



}
