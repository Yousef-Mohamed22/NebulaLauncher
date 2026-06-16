#include <Plugin/PluginContext.hpp>
#include <Plugin/InstallationType.hpp>
#include <Core/Result.hpp>
#include <Core/Array.hpp>
#include <Platform/Process.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

const nbl::InstallationType* nbl::PluginContext::getInstallationType(StringView id) const {
    for(const auto& type : types()) {
        if(type.id == id) {
            return &type;
        }
    }
    return nullptr;
}

nbl::Result nbl::PluginContext::registerInstallationType(const InstallationType& type) {

    InstallationType* registered = nullptr;
    NBL_CHECK(_mArena.allocate(&registered));

    _mNumTypes++;

    *registered = type;
    return nbl::Result::SUCCESS;
}

nbl::Span<nbl::InstallationType> nbl::PluginContext::types() const {

    void* typesStart;
    if(_mArena.get(_mTypesOffset, &typesStart).error()) {
        return {};
    }

    return nbl::Span(
        static_cast<nbl::InstallationType*>(typesStart), 
        _mNumTypes
    );
}


nbl::Result nbl::PluginContext::create(StringView databasePath, PluginContext* ctx) {

    NBL_CHECK(nbl::Arena::create(1024 * 1024 * 16, &ctx->_mArena));

    {
        nbl::ArenaFrame frame(&ctx->_mArena);
        NBL_CHECK(nbl::StringBuilder(&frame)
                .append(databasePath)
                .buildNullTerminated(&ctx->_mDatabasePath));

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
    usize padding = nbl::align(offset, sizeof(void*)) - ctx->_mArena.index();

    void* alloc;
    NBL_CHECK(ctx->_mArena.push(padding, &alloc));
    memset(alloc, 0, padding);

    ctx->_mTypesOffset = offset + padding - 1;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::PluginContext::runCommand(i64 installationId, StringView command, ProcessHandle* handle) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringView idString;
    NBL_CHECK(nbl::StringBuilder(&frame).append(installationId).buildNullTerminated(&idString));

    nbl::Array args = {
        idString,
        _mDatabasePath,
        command
    };

    nbl::ProcessHandle::spawn(_mWrapperPath, args, handle, nbl::ProcessCreateFlag::CAPTURE_OUTPUT);

    return nbl::Result::SUCCESS;
}
