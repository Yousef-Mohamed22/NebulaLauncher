#include <Plugin/InstallationType.hpp>
#include <Core/Array.hpp>
#include <Core/Result.hpp>
#include <Platform/Logging.hpp>

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
        nbl::ProcessTracker* tracker, 
        void* userData) {

    return tracker->run(ctx, fields[0].stringValue, fields[1].stringValue);
}

nbl::Result _getFolder(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        i64 installationId, 
        nbl::StringView* outFolder,
        void* userData) {

    *outFolder = fields[1].stringValue;

    return nbl::Result::SUCCESS;
}

}

nbl::InstallationType nbl::InstallationType::COMMAND = {
    .id = "command",
    .name = "Command",
    .fields = _fields,
    .launchFunc = {
        .pfnLaunchInstallation = _launch,
        .userData = nullptr
    },
    .getFolderFunc = {
        .pfnGetFolder = _getFolder,
        .userData = nullptr
    }
};

