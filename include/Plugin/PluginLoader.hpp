#pragma once

#include <Core/String.hpp>

#include <Plugin/Field.hpp>
#include <Plugin/Registry.hpp>
#include <Platform/Process.hpp>
#include <Platform/DynamicLibrary.hpp>
#include <Platform/Allocator.hpp>
#include <Platform/Pool.hpp>

namespace nbl {

struct PluginMeta {
    StringView id;
    StringView name;
    StringView version;
    StringView description;
};

struct LoadedPlugin {
    PluginMeta meta;
    DynamicLibrary lib;
    Span<Field> fields;
    void(*pfnUpdateCallback)(Span<Field> fields, Span<FieldValue> newValues, void* userData); 
    void* userData;
};

class PluginContext;

class PluginLoader {
public:

    ~PluginLoader() { 
        awaitLoaded(); 
        _unloadAll();
    }

    Result beginLoadAsync();
    Result awaitLoaded();

    Span<LoadedPlugin> loadedPlugins() { 
        return {
            .data = static_cast<LoadedPlugin*>(_mMetaArena.begin()),
            .length = _mMetaArena.index() / sizeof(LoadedPlugin)
        };
    }

    static Result create(PluginContext* context, StringView pluginFolder, PluginLoader* out);

private:

    Result _loadAll();
    Result _load(StringView folder, Registry::Builder* registrations);

    void _unloadAll();

    Arena _mMetaArena;
    Arena _mDataArena;
    Thread _mLoadThread;

    StringView _mPluginFolder;
    PluginContext* _pContext;

};


}
