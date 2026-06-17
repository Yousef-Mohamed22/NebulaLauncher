#pragma once

#include <Plugin/PluginContext.hpp>
#include <Core/String.hpp>
#include "Platform/DynamicLibrary.hpp"

namespace nbl {

struct LoadedPlugin {
    StringView id;
    StringView name;
    StringView version;
    StringView description;
    DynamicLibrary lib;
};

class PluginLoader {
public:

    Result loadAll();

    static Result create(PluginContext* context, StringView pluginFolder, PluginLoader* out);

private:

    Result _load(StringView folder);

    Arena _mMetaArena;
    Arena _mDataArena;

    StringView _mPluginFolder;
    PluginContext* _pContext;

};


}
