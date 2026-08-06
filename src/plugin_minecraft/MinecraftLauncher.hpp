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

    static Result init(StringView rootFolder, MinecraftLauncher* launcher);

private:

    StringView _mRootFolder;
    HttpClient _mHttpClient;
    Arena _mDataArena;
    Optional<JsonObject> _mVersionManifest;


};



}
