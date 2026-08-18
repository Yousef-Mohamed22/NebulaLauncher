#include <Plugin/PluginInitializer.hpp>
#include <Plugin/InstallationType.hpp>
#include <Platform/Logging.hpp>
#include <Core/Array.hpp>
#include "Core/String.hpp"
#include "Plugin/AccountProvider.hpp"
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

void _suggest(nbl::StringView current, std::function_ref<void(nbl::StringView)> suggestionConsumer, void* userData) {

    suggestionConsumer("Hello");
    suggestionConsumer("World");

}

nbl::Field _listTemplate = {
    .type = nbl::FieldType::STRING,
    .key = "",
    .name = "",
    .defaultValue = { .stringValue = "Test" }
};

nbl::Array _structFields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "string",
        .name = "String"
    },
    nbl::Field {
        .type = nbl::FieldType::INTEGER,
        .key = "int",
        .name = "Int",
        .integerData = {
            .min = -10,
            .max = 10
        }
    }
};

nbl::Array _fields = {
    nbl::Field {
        .type = nbl::FieldType::STRING,
        .key = "string",
        .name = "String",
        .stringData = {
            .pfnSuggest = _suggest,
            .userData = nullptr
        }
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
    nbl::Field {
        .type = nbl::FieldType::ACCOUNT,
        .key = "account",
        .name = "Account",
        .accountData = {
            .accountProvider = "test",
            .showDefaultOption = false
        },
        .defaultValue = {
            .accountValue = {
                .id = 0,
                .isDefault = true
            }
        }
    },
    nbl::Field {
        .type = nbl::FieldType::ACCOUNT,
        .key = "account_default",
        .name = "Account (Default)",
        .accountData = {
            .accountProvider = "test",
            .showDefaultOption = true
        },
        .defaultValue = {
            .accountValue = {
                .id = 0,
                .isDefault = true
            }
        }
    },
    nbl::Field {
        .type = nbl::FieldType::LIST,
        .key = "list",
        .name = "List",
        .listData = {
            .templateField = &_listTemplate,
            .min = 0,
            .max = 3
        },
        .defaultValue = {
            .compositeValue = {}
        }
    },
    nbl::Field {
        .type = nbl::FieldType::STRUCT,
        .key = "struct",
        .name = "Struct",
        .structData = {
            .fields = _structFields
        },
        .defaultValue = {
            .compositeValue = {}
        }
    }
};

struct _TestAccount {
    nbl::AccountInfo info;
    bool isLoggedIn;
};

nbl::Array<_TestAccount, 3> _infos = {
    _TestAccount{ .info = nbl::AccountInfo{ .userName = "Test1", .icon = "", .id = 1 }, .isLoggedIn = false },
    _TestAccount{ .info = nbl::AccountInfo{ .userName = "Test2", .icon = "", .id = 2 }, .isLoggedIn = false },
    _TestAccount{ .info = nbl::AccountInfo{ .userName = "Test3", .icon = "", .id = 3 }, .isLoggedIn = false },
};

u32 _firstFree = 0;


nbl::AccountProvider _provider {
    .id = "test",
    .name = "Test",
    .type = nbl::AccountProvider::Type::BASIC,
    .maxAccounts = 3,
    .basicData = {
        .pfnLogin = [](nbl::StringView username, nbl::StringView password, nbl::LoginTracker* tracker, void* userData) -> nbl::Result {

            if(username.length == 0) return nbl::Result::INVALID_ARGUMENT;
            if(_firstFree == 3) return nbl::Result::INVALID_STATE;

            u32 id = _firstFree;
            _firstFree = _infos[id].info.id;
            _infos[id].info.id = id;
            _infos[id].isLoggedIn = true;

            tracker->postStatus("3...");
            nbl::sleep(1000);
            tracker->postStatus("2...");
            nbl::sleep(1000);
            tracker->postStatus("1...");
            nbl::sleep(1000);

            tracker->finishTracking(true, {});
            return nbl::Result::SUCCESS;
        },
        .pfnLogout = [](u32 accountId, void* userData) -> nbl::Result {
            if(accountId > 2) return nbl::Result::INVALID_STATE;

            _infos[accountId].isLoggedIn = false;
            if(accountId < _firstFree) {
                _infos[accountId].info.id = _firstFree;
                _firstFree = accountId;
            }

            return nbl::Result::SUCCESS;
        }
    },
    .accountInfo = {
        .pfnGet = [](u32 accountId, nbl::AccountInfo* info, void* userData) -> nbl::Result {
            nbl::log(nbl::LogLevel::INFO, "Get: ", accountId);
            if(accountId == 0 || accountId > 3) return nbl::Result::NOT_FOUND;
            if(!_infos[accountId].isLoggedIn) return nbl::Result::NOT_FOUND;
            *info = _infos[accountId - 1].info;
            return nbl::Result::SUCCESS;
        },
        .pfnEnumerate = [](nbl::AccountCallback callback, void* userData) -> nbl::Result {
            for(_TestAccount& acc : _infos) {
                if(acc.isLoggedIn) {
                    callback(acc.info);
                }
            }
            return nbl::Result::SUCCESS;
        }
    }
};


nbl::Result _launch(
        const nbl::InstallationType* type, 
        nbl::PluginContext* ctx, 
        nbl::Span<nbl::FieldValue> fields, 
        i64 installationId, 
        nbl::ProcessTracker* handle, 
        void* userData) {

    nbl::log(nbl::LogLevel::INFO, "String: ", fields.data[0].stringValue);
    nbl::log(nbl::LogLevel::INFO, "Integer: ", fields.data[1].integerValue);
    nbl::log(nbl::LogLevel::INFO, "Float: ", fields.data[2].floatValue);
    nbl::log(nbl::LogLevel::INFO, "Boolean: ", fields.data[3].booleanValue);
    nbl::log(nbl::LogLevel::INFO, "Enum: ", _name(fields.data[4].enumValue, nullptr));
    nbl::log(nbl::LogLevel::INFO, "Date: ", fields.data[5].uintegerValue);

    nbl::log(nbl::LogLevel::INFO, "Account: ", fields.data[6].accountValue.id);
    nbl::log(nbl::LogLevel::INFO, "Account (Default): ", fields.data[7].accountValue.id);

    nbl::log(nbl::LogLevel::INFO, "List.length: ", fields.data[8].compositeValue.length);
    for(usize i = 0 ; i < fields.data[8].compositeValue.length ; i++) {
        nbl::log(nbl::LogLevel::INFO, "List[", i, "]: ", fields.data[8].compositeValue[i].stringValue);
    }
    nbl::log(nbl::LogLevel::INFO, "Struct.String: ", fields.data[9].compositeValue[0].stringValue);
    nbl::log(nbl::LogLevel::INFO, "Struct.Int: ", fields.data[9].compositeValue[1].integerValue);

    ctx->pushNotification(nbl::NotificationType::INFO, "Hello", "Hello from test plugin");
    handle->markComplete();

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
    context->registerAccountProvider(_provider);

    return 0;
}
