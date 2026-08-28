#include <Plugin/PluginInitContext.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/AccountProvider.hpp>

nbl::Result nbl::PluginInitContext::registerInstallationType(const InstallationType* type) {
    PluginRegistries::Registry* reg;
    NBL_CHECK(_pAllocator->allocate(&reg));
    *reg = PluginRegistries::INSTALLATION_TYPE;

    const InstallationType** it;
    NBL_CHECK(_pAllocator->allocate(&it));
    *it = type;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginInitContext::registerAccountProvider(const AccountProvider* type) {
    PluginRegistries::Registry* reg;
    NBL_CHECK(_pAllocator->allocate(&reg));
    *reg = PluginRegistries::ACCOUNT_PROVIDER;

    const AccountProvider** ap;
    NBL_CHECK(_pAllocator->allocate(&ap));
    *ap = type;
    return nbl::Result::SUCCESS;
}



nbl::Result nbl::PluginInitContext::create(LinearAllocator* allocator, PluginMeta* meta, StringView databasePath, StringView pluginDataPath, PluginInitContext* outCtx) {

    outCtx->_pAllocator = allocator;
    outCtx->_pMeta = meta;
    outCtx->_mDatabasePath = databasePath;
    outCtx->_mPluginDataPath = pluginDataPath;

    return nbl::Result::SUCCESS;
}
