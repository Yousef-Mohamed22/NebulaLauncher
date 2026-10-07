#pragma once

#include <Core/String.hpp>
#include <Platform/Allocator.hpp>
#include <Platform/StringBuilder.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/Field.hpp>
#include <Data/Database.hpp>
#include <Plugin/PluginLoader.hpp>

namespace nbl {

class PluginInitContext {
public:

    template<typename T>
    inline Result registerObject(T* object) {
        return _pBuilder->add(object);
    }

    inline const PluginMeta* getMeta() const { return &_pPlugin->meta; }

    inline StringView pluginDataPath() const { return _pContext->pluginDataPath(); }
    inline StringView installFolder() const { return _mInstallFolder; }

    Result getStoredFieldsObject(JsonObject* outValues, StringView* outVersion, LinearAllocator* allocator);
    Result getStoredFieldsVersion(StringView* outVersion, LinearAllocator* allocator);
    Result getStoredFieldValues(Span<Field> fields, Span<FieldValue>* outValues, LinearAllocator* allocator);
    Result storeFieldValues(Span<Field> fields, Span<FieldValue> values);
    Result setFields(Span<Field> fields, void(*pfnUpdateCallback)(Span<Field> fields, Span<FieldValue> newValues, void* userData), void* userData);

    static Result create(Registry::Builder* builder, LoadedPlugin* plugin, PluginContext* context, StringView installFolder, PluginInitContext* outCtx);

private:

    bool _verifyId(StringView id);

    Registry::Builder* _pBuilder;
    
    LoadedPlugin* _pPlugin;
    PluginContext* _pContext;

    StringView _mInstallFolder;

};

}
