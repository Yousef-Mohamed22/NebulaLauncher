#include <Plugin/PluginContext.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Plugin/MetadataProvider.hpp>
#include <Core/Result.hpp>
#include <Core/Array.hpp>
#include <Core/Math.hpp>
#include <Platform/Process.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>
#include "Data/Database.hpp"
#include "Data/Model.hpp"


const nbl::InstallationType* nbl::PluginContext::getInstallationType(StringView id) const {
    for(const auto& type : types()) {
        if(type->id == id) {
            return type;
        }
    }
    return nullptr;
}

nbl::ConstSpan<const nbl::InstallationType*> nbl::PluginContext::types() const {
    return nbl::ConstSpan(
        static_cast<const InstallationType**>(_pStarts[PluginRegistries::INSTALLATION_TYPE]),
        _mCounts[PluginRegistries::INSTALLATION_TYPE]
    );
}

const nbl::AccountProvider* nbl::PluginContext::getAccountProvider(StringView id) const {
    for(const auto& prov : accountProviders()) {
        if(prov->id == id) {
            return prov;
        }
    }
    return nullptr;
}

nbl::ConstSpan<const nbl::AccountProvider*> nbl::PluginContext::accountProviders() const {
    return nbl::ConstSpan(
        static_cast<const AccountProvider**>(_pStarts[PluginRegistries::ACCOUNT_PROVIDER]),
        _mCounts[PluginRegistries::ACCOUNT_PROVIDER]
    );
}

const nbl::MetadataProvider* nbl::PluginContext::getMetadataProvider(StringView id) const {
    for(const auto& prov : metadataProviders()) {
        if(prov->id == id) {
            return prov;
        }
    }
    return nullptr;
}
nbl::ConstSpan<const nbl::MetadataProvider*> nbl::PluginContext::metadataProviders() const {
    return nbl::ConstSpan(
        static_cast<const MetadataProvider**>(_pStarts[PluginRegistries::METADATA_PROVIDER]),
        _mCounts[PluginRegistries::METADATA_PROVIDER]
    );
}


nbl::Result nbl::PluginContext::create(StringView databasePath, StringView pluginDataPath, NotificationConsumer consumer, PluginContext* ctx) {

    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 16, &ctx->_mArena));

    {
        nbl::ArenaFrame frame(&ctx->_mArena);
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append(databasePath)
                .buildNullTerminated(&ctx->_mDatabasePath));

        NBL_CHECK(nbl::StringBuilder(&frame)
                .append(pluginDataPath)
                .buildNullTerminated(&ctx->_mPluginDataPath));

        StringView binaryPath = getBinaryPath();
        usize index;
        NBL_CHECK(binaryPath.findLast('/', &index));
        StringView binaryFolder = binaryPath.slice(0, index + 1);
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append(binaryFolder)
                .append("Nebula_Wrapper")
                .buildNullTerminated(&ctx->_mWrapperPath));

        frame.release();
    }

    usize offset = ctx->_mArena.index() + 1;
    usize padding = nbl::alignUp(offset, sizeof(void*)) - ctx->_mArena.index();

    void* alloc;
    NBL_CHECK(ctx->_mArena.push(padding, &alloc));
    memset(alloc, 0, padding);


    NBL_CHECK(nbl::DatabaseConnection::open(ctx->_mDatabasePath, &ctx->_mConnection));
    NBL_CHECK(nbl::initDatabase(&ctx->_mConnection));



    ctx->_mNotificationConsumer = consumer;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginContext::runCommand(i64 installationId, StringView command, StringView workingDir, ProcessHandle* handle) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView idString;
    NBL_CHECK(nbl::StringBuilder(&frame).append(installationId).buildNullTerminated(&idString));

    nbl::Array args = {
        idString,
        _mDatabasePath,
        command
    };

    nbl::ProcessHandle::spawn(_mWrapperPath, args, handle, {
        .outputMode = ProcessOutputMode::PIPE,
        .workingDir = workingDir
    });

    return nbl::Result::SUCCESS;
}
