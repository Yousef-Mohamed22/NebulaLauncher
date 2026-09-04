#pragma once

#include <Plugin/PluginContext.hpp>
#include <Core/String.hpp>

#include <Platform/DynamicLibrary.hpp>
#include <Plugin/Field.hpp>
#include <Platform/Allocator.hpp>

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


class PluginLoader {
public:

    Result loadAll();

    Span<LoadedPlugin> loadedPlugins() { 
        return _mPluginRefs;
    }

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
