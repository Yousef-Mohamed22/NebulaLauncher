#include <Plugin/InstallationType.hpp>
#include <Core/Array.hpp>

namespace {

nbl::Array _fields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "command",
        .name = "Command"
    },
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "working_dir",
        .name = "Working Directory"
    }
};

nbl::Result _launch(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        i64 installationId, 
        nbl::ProcessHandle* handle, 
        void* userData) {

    return ctx->runCommand(installationId, fields[0].stringValue, fields[1].stringValue, handle);
}

}

nbl::InstallationType nbl::InstallationType::COMMAND = {
    .id = "command",
    .name = "Command",
    .fields = _fields,
    .launchFunc = {
        .pfnLaunchInstallation = _launch,
        .userData = nullptr
    }
};

