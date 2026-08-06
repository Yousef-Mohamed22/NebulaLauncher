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



nbl::Result nbl::encodeFieldValues(const InstallationType* type, Span<FieldValue> values, Writer writer) {

    return JsonWriter(writer).writeObject([&](auto obj) {

        for(usize i = 0 ; i < type->fields.length ; i++) {

            const Field& field = type->fields[i];
            switch(field.type) {
                case nbl::FieldType::STRING: 
                    obj.add(field.key, values[i].stringValue);
                    break;
                case nbl::FieldType::FLOAT:
                    obj.add(field.key, values[i].floatValue);
                    break;
                case nbl::FieldType::INTEGER:
                    obj.add(field.key, values[i].integerValue);
                    break;
                case nbl::FieldType::BOOLEAN:
                    obj.add(field.key, values[i].booleanValue);
                    break;
                case nbl::FieldType::ENUM: {
                    StringView name = field.enumData.name(values[i].enumValue);
                    obj.add(field.key, name);
                    break;
                }
                case nbl::FieldType::DATE: 
                    obj.add(field.key, values[i].uintegerValue);
                    break;
            }
        }
    });   
}

nbl::Result nbl::decodeFieldValues(const InstallationType* type, JsonObject obj, Span<FieldValue> outValues) {

    if(outValues.length < type->fields.length) return nbl::Result::INVALID_ARGUMENT;

    for(usize i = 0 ; i < type->fields.length ; i++) {

        const Field& field = type->fields[i];

        const JsonElement* val = obj.get(field.key);
        if(val == nullptr) {
            outValues[i] = field.defaultValue;
        } else {
            switch(field.type) {
                case nbl::FieldType::STRING: 
                    if(val->type != nbl::JsonType::STRING) return nbl::Result::INVALID_ARGUMENT;
                    outValues[i] = nbl::FieldValue { .stringValue = val->string };
                    break;
                case nbl::FieldType::FLOAT:
                    if(val->type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
                    outValues[i] = nbl::FieldValue { .floatValue = val->number.asFloat() };
                    break;
                case nbl::FieldType::INTEGER:
                    if(val->type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
                    outValues[i] = nbl::FieldValue { .integerValue = val->number.asInt() };
                    break;
                case nbl::FieldType::BOOLEAN:
                    if(val->type != nbl::JsonType::BOOLEAN) return nbl::Result::INVALID_ARGUMENT;
                    outValues[i] = nbl::FieldValue { .booleanValue = val->boolean };
                    break;
                case nbl::FieldType::ENUM: {
                    if(val->type != nbl::JsonType::STRING) return nbl::Result::INVALID_ARGUMENT;
                    u32 index = field.enumData.index(val->string);
                    outValues[i] = nbl::FieldValue { .enumValue = index };
                    break;
                }
                case nbl::FieldType::DATE: 
                    if(val->type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
                    outValues[i] = nbl::FieldValue { .uintegerValue = val->number.asUint() };
                    break;
            }
        }
    }

    outValues.length = type->fields.length;
    return nbl::Result::SUCCESS;
}
