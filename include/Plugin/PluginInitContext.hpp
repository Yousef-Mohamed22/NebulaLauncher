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

    inline Result registerInstallationType(const InstallationType* type) {
        return _register(PluginRegistries::Registry::INSTALLATION_TYPE, type);
    }
    Result registerAccountProvider(const AccountProvider* provider) {
        return _register(PluginRegistries::Registry::ACCOUNT_PROVIDER, provider);
    }
    Result registerMetadataProvider(const MetadataProvider* provider) {
        return _register(PluginRegistries::Registry::METADATA_PROVIDER, provider);
    }
    Result registerGameProvider(const GameProvider* provider) {
        return _register(PluginRegistries::Registry::GAME_PROVIDER, provider);
    }

    inline const PluginMeta* getMeta() const { return &_pPlugin->meta; }

    inline StringView databasePath() const { return _mDatabasePath; }
    inline StringView pluginDataPath() const { return _mPluginDataPath; }

    Result getStoredFieldsObject(JsonObject* outValues, StringView* outVersion, LinearAllocator* allocator);
    Result getStoredFieldsVersion(StringView* outVersion, LinearAllocator* allocator);
    Result getStoredFieldValues(Span<Field> fields, Span<FieldValue>* outValues, LinearAllocator* allocator);
    Result storeFieldValues(Span<Field> fields, Span<FieldValue> values);
    Result setFields(Span<Field> fields, void(*pfnUpdateCallback)(Span<Field> fields, Span<FieldValue> newValues, void* userData), void* userData);

    static Result create(LinearAllocator* allocator, LoadedPlugin* plugin, DatabaseConnection* connection, StringView databasePath, StringView pluginDataPath, PluginInitContext* outCtx);

private:

    bool _verifyId(StringView id);
    Result _register(PluginRegistries::Registry reg, const void* value);

    LinearAllocator* _pAllocator;
    LoadedPlugin* _pPlugin;
    DatabaseConnection* _pConnection;

    StringView _mDatabasePath;
    StringView _mPluginDataPath;

};

}
