#include <Plugin/PluginContext.hpp>
#include <Plugin/InstallationType.hpp>
#include <Plugin/AccountProvider.hpp>
#include <Plugin/MetadataProvider.hpp>
#include <Plugin/GameProvider.hpp>
#include <Core/Result.hpp>
#include <Core/Array.hpp>
#include <Core/Math.hpp>
#include <Platform/Process.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>
#include <Data/Database.hpp>
#include <Data/Model.hpp>


nbl::Result nbl::PluginContext::create(StringView databasePath, StringView pluginFolder, StringView pluginDataPath, NotificationConsumer consumer, PluginContext* ctx) {

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

    NBL_CHECK(nbl::PluginLoader::create(ctx, pluginFolder, &ctx->_mLoader));

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
