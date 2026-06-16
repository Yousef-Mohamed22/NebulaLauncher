#pragma once

#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Plugin/PluginContext.hpp>

namespace nbl {

enum FieldType {
    STRING,
    INTEGER,
    FLOAT,
    ENUM
};

struct FieldValue {
    union {
        StringView stringValue;
        i64 integerValue;
        f64 floatValue;
        u32 enumValue;
    };
};

struct Field {
    FieldType type;
    StringView key;
    StringView name;
    union {
        struct {
            i64 min;
            i64 max;
        } integerData;
        struct {
            f64 min;
            f64 max;
        } floatData;
        struct {
            u32 (*pfnIndex)(StringView name, void* userData);
            StringView (*pfnName)(u32 index, void* userData);
            u32 (*pfnCount)(void* userData);
            void* userData;

            u32 count() { return pfnCount(userData); }
            u32 index(StringView name) { return pfnIndex(name, userData); }
            StringView name(u32 index) { return pfnName(index, userData); }
        } enumData;
    };
    FieldValue defaultValue;
    bool hasDefaultValue;
};

struct InstallationType;
struct LaunchInstallationFunction {
    Result (*pfnLaunchInstallation)(
            const InstallationType* type,
            PluginContext* ctx, 
            nbl::Span<FieldValue> fields, 
            i64 installationId,
            ProcessHandle* handle, 
            void* userData);
    void* userData;
};

struct InstallationType {
    StringView id;
    StringView name;
    Span<Field> fields;
    LaunchInstallationFunction launchFunc;

    inline Result launch(PluginContext* ctx, nbl::Span<FieldValue> fields, i64 installationId, ProcessHandle* handle) const {
        return launchFunc.pfnLaunchInstallation(this, ctx, fields, installationId, handle, launchFunc.userData);
    }

    static InstallationType COMMAND;
};

}
