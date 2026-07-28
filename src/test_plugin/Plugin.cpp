#include <Plugin/PluginInitializer.hpp>
#include <Plugin/InstallationType.hpp>
#include <Platform/Logging.hpp>
#include <Core/Array.hpp>
#include "Plugin/PluginContext.hpp"

namespace {

enum _TestEnum : u32 {
    ONE,
    TWO,
    THREE,
    _END
};

u32 _count(void* userData) { 
    return _END;
}

nbl::StringView _name(u32 index, void* userData) {
    switch(index) {
        case ONE: return "one";
        case TWO: return "two";
        case THREE: return "three";
    }
    return "";
}

u32 _index(nbl::StringView name, void* userData) {
    if(name == "one") return ONE;
    if(name == "two") return TWO;
    if(name == "three") return THREE;
    return 0;
}

nbl::Array _fields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "string",
        .name = "String"
    },
    nbl::Field {
        .type = nbl::FieldType::INTEGER,
        .key = "integer",
        .name = "Integer",
        .integerData = {
            .min = 0,
            .max = 10
        }
    },
    nbl::Field {
        .type = nbl::FieldType::FLOAT,
        .key = "float",
        .name = "Float",
        .floatData = {
            .min = -100.0f,
            .max = 100.0f
        }
    },
    nbl::Field {
        .type = nbl::FieldType::BOOLEAN,
        .key = "boolean",
        .name = "Boolean",
        .defaultValue = {
            .booleanValue = true
        }
    },
    nbl::Field {
        .type = nbl::FieldType::ENUM,
        .key = "enum",
        .name = "Enum",
        .enumData = {
            .pfnIndex = _index,
            .pfnName = _name,
            .pfnCount = _count,
            .userData = nullptr
        },
        .defaultValue = {
            .enumValue = ONE
        }
    },
    nbl::Field {
        .type = nbl::FieldType::DATE,
        .key = "date",
        .name = "Date",
        .defaultValue = {
            .uintegerValue = 0
        }
    },
};

nbl::Result _launch(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        i64 installationId, 
        nbl::ProcessHandle* handle, 
        void* userData) {

    nbl::log(nbl::LogLevel::INFO, "String: ", fields.data[0].stringValue);
    nbl::log(nbl::LogLevel::INFO, "Integer: ", fields.data[1].integerValue);
    nbl::log(nbl::LogLevel::INFO, "Float: ", fields.data[2].floatValue);
    nbl::log(nbl::LogLevel::INFO, "Boolean: ", fields.data[3].booleanValue);
    nbl::log(nbl::LogLevel::INFO, "Enum: ", _name(fields.data[4].enumValue, nullptr));

    return nbl::Result::SUCCESS;
}


nbl::InstallationType _type = {
    .id = "test",
    .name = "Test",
    .fields = _fields,
    .launchFunc = {
        .pfnLaunchInstallation = _launch,
        .userData = nullptr
    }
};

}


extern "C" i32 initPlugin(nbl::PluginContext *context) {

    nbl::log(nbl::LogLevel::INFO, "Hello from plugin");

    context->registerInstallationType(_type);

    return 0;
}
