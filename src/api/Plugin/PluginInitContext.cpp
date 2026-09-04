#include <Plugin/PluginInitContext.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Plugin/MetadataProvider.hpp>


nbl::Result nbl::PluginInitContext::_register(nbl::PluginRegistries::Registry registry, const void* value) {

    PluginRegistries::Registry* reg;
    NBL_CHECK(_pAllocator->allocate(&reg));
    *reg = registry;

    const void** val;
    NBL_CHECK(_pAllocator->allocate(&val));
    *val = value;

    return nbl::Result::SUCCESS;
}



nbl::Result nbl::PluginInitContext::create(LinearAllocator* allocator, PluginMeta* meta, StringView databasePath, StringView pluginDataPath, PluginInitContext* outCtx) {

    outCtx->_pAllocator = allocator;
    outCtx->_pMeta = meta;
    outCtx->_mDatabasePath = databasePath;
    outCtx->_mPluginDataPath = pluginDataPath;

    return nbl::Result::SUCCESS;
}
