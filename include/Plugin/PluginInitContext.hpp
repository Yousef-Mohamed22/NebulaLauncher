#pragma once

#include <Core/String.hpp>
#include <Platform/Allocator.hpp>
#include <Plugin/PluginContext.hpp>

namespace nbl {

struct PluginMeta {
    StringView id;
    StringView name;
    StringView version;
    StringView description;
};

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

    inline const PluginMeta* getMeta() const { return _pMeta; }

    inline StringView databasePath() const { return _mDatabasePath; }
    inline StringView pluginDataPath() const { return _mPluginDataPath; }

    static Result create(LinearAllocator* allocator, PluginMeta* meta, StringView databasePath, StringView pluginDataPath, PluginInitContext* outCtx);

private:

    Result _register(PluginRegistries::Registry reg, const void* value);

    LinearAllocator* _pAllocator;
    PluginMeta* _pMeta;
    StringView _mDatabasePath;
    StringView _mPluginDataPath;

};

}
