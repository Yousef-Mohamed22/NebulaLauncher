#pragma once

#include <Core/String.hpp>
#include <Core/Span.hpp>
#include <Codec/Json.hpp>
#include <Plugin/PluginContext.hpp>

namespace nbl {

enum FieldType {
    STRING,
    INTEGER,
    FLOAT,
    BOOLEAN,
    ENUM
};

struct FieldValue {
    union {
        StringView stringValue;
        i64 integerValue;
        f64 floatValue;
        bool booleanValue;
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

            u32 count() const { 
                if(pfnCount) { 
                    return pfnCount(userData); 
                } 
                return 0;
            }
            u32 index(StringView name) const { 
                if(pfnIndex) {
                    return pfnIndex(name, userData); 
                }
                return 0;
            }
            StringView name(u32 index) const { 
                if(pfnIndex) {
                    return pfnName(index, userData);
                }
                return "";
            }
        } enumData;
    };
    FieldValue defaultValue;
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

    inline Result launch(PluginContext* ctx, nbl::Span<FieldValue> fields, i64 installationId, ProcessHandle* handle) const {
        return launchFunc.pfnLaunchInstallation(this, ctx, fields, installationId, handle, launchFunc.userData);
    }

    inline Result getFolder(PluginContext* ctx, nbl::Span<FieldValue> fields, i64 installationId, StringView* outFolder) const {
        return getFolderFunc.pfnGetFolder(this, ctx, fields, installationId, outFolder, getFolderFunc.userData);
    }

    static InstallationType COMMAND;
};


Result encodeFieldValues(const InstallationType* type, Span<FieldValue> values, ArenaFrame* frame, JsonObject* outObject);

Result decodeFieldValues(const InstallationType* type, JsonObject obj, Span<FieldValue> values);

}
