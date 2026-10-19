/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file Plugin/PluginLoader.hpp
 * \author jwall64
 * \brief Interface for loading plugins from disk.
 */

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
