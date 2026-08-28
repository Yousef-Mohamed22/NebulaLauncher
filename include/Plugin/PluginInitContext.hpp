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

    Result registerInstallationType(const InstallationType* type);
    Result registerAccountProvider(const AccountProvider* type);

    inline const PluginMeta* getMeta() const { return _pMeta; }

    inline StringView databasePath() const { return _mDatabasePath; }
    inline StringView pluginDataPath() const { return _mPluginDataPath; }

    static Result create(LinearAllocator* allocator, PluginMeta* meta, StringView databasePath, StringView pluginDataPath, PluginInitContext* outCtx);

private:

    LinearAllocator* _pAllocator;
    PluginMeta* _pMeta;
    StringView _mDatabasePath;
    StringView _mPluginDataPath;

};

}
