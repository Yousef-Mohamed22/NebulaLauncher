#pragma once

#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Codec/Json.hpp>
#include <Plugin/PluginContext.hpp>
#include <Plugin/ProcessTracker.hpp>
#include <Plugin/Field.hpp>

namespace nbl {

struct InstallationType;
struct LaunchInstallationFunction {
    Result (*pfnLaunchInstallation)(
            const InstallationType* type,
            PluginContext* ctx, 
            nbl::Span<FieldValue> fields, 
            i64 installationId,
            ProcessTracker* tracker, 
            void* userData);
    void* userData;
};
struct GetFolderFunction {
    Result (*pfnGetFolder)(
            const InstallationType* type,
            PluginContext* ctx, 
            nbl::Span<FieldValue> fields, 
            i64 installationId,
            StringView* outFolder,
            void* userData);
    void* userData;
};

struct InstallationType {
    StringView id;
    StringView name;
    Span<Field> fields;
    LaunchInstallationFunction launchFunc;
    GetFolderFunction getFolderFunc;

    inline Result launch(PluginContext* ctx, nbl::Span<FieldValue> fields, i64 installationId, ProcessTracker* handle) const {
        return launchFunc.pfnLaunchInstallation(this, ctx, fields, installationId, handle, launchFunc.userData);
    }

    inline Result getFolder(PluginContext* ctx, nbl::Span<FieldValue> fields, i64 installationId, StringView* outFolder) const {
        return getFolderFunc.pfnGetFolder(this, ctx, fields, installationId, outFolder, getFolderFunc.userData);
    }

    static InstallationType COMMAND;
};

}
