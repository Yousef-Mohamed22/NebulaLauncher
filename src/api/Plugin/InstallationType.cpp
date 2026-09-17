#include <Plugin/InstallationType.hpp>
#include <Core/Array.hpp>
#include <Core/Result.hpp>
#include <Platform/Logging.hpp>

namespace {

nbl::Array _fields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "command",
        .name = "Command",
        .stringData = {
            .applyGamePlaceholders = true,
            .strictPlaceholders = false
        }
    },
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "working_dir",
        .name = "Working Directory",
        .stringData = {
            .applyGamePlaceholders = true,
            .strictPlaceholders = true
        }
    }
};

nbl::Result _launch(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        const nbl::InstallationMeta& meta, 
        nbl::ProcessTracker* tracker, 
        void* userData) {

    return tracker->run(ctx, fields[0].stringValue.resolved, fields[1].stringValue.resolved);
}

nbl::Result _getFolder(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        const nbl::InstallationMeta& meta, 
        nbl::StringView* outFolder,
        void* userData) {

    *outFolder = fields[1].stringValue.resolved;
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

