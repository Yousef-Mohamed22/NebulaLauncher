#pragma once

#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Codec/Json.hpp>
#include <Data/Model.hpp>
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
            const nbl::InstallationMeta& meta,
            ProcessTracker* tracker, 
            void* userData);
    void* userData;
};
struct GetFolderFunction {
    Result (*pfnGetFolder)(
            const InstallationType* type,
            PluginContext* ctx, 
            nbl::Span<FieldValue> fields, 
            const nbl::InstallationMeta& meta,
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

    inline Result launch(PluginContext* ctx, nbl::Span<FieldValue> fields, const nbl::InstallationMeta& meta, ProcessTracker* handle) const {
        if(launchFunc.pfnLaunchInstallation == nullptr) return nbl::Result::PLUGIN_ERROR;
        return launchFunc.pfnLaunchInstallation(this, ctx, fields, meta, handle, launchFunc.userData);
    }

    inline Result getFolder(PluginContext* ctx, nbl::Span<FieldValue> fields, const nbl::InstallationMeta& meta, StringView* outFolder) const {
        if(getFolderFunc.pfnGetFolder == nullptr) return nbl::Result::PLUGIN_ERROR;
        return getFolderFunc.pfnGetFolder(this, ctx, fields, meta, outFolder, getFolderFunc.userData);
    }

    static InstallationType COMMAND;
};

}
