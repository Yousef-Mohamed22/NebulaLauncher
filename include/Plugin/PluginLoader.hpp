#pragma once

#include <Plugin/PluginContext.hpp>
#include <Core/String.hpp>

#include <Platform/DynamicLibrary.hpp>
#include <Plugin/PluginInitContext.hpp>
#include "Platform/Allocator.hpp"

namespace nbl {

struct LoadedPlugin {
    PluginMeta meta;
    DynamicLibrary lib;
};


class PluginLoader {
public:

    Result loadAll();

    static Result create(PluginContext* context, StringView pluginFolder, PluginLoader* out);

private:

    Result _load(StringView folder, LinearAllocator* registrations);

    Arena _mMetaArena;
    Arena _mDataArena;
    Span<LoadedPlugin> _mPluginRefs;

    StringView _mPluginFolder;
    PluginContext* _pContext;

};


}
