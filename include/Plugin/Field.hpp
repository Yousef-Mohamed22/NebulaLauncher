#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>
#include <Utility/Placeholder.hpp>

#include <functional>

namespace nbl {


enum class FieldType : u8 {
    STRING        = 0,  // A text box. Stored as a string.
    INTEGER       = 1,  // A number input box that only allows integers. Stored as a number.
    FLOAT         = 2,  // A number input box. Stored as a number.
    BOOLEAN       = 3,  // A checkbox. Stored as a boolean
    ENUM          = 4,  // A combobox that displays certain values. Stored as a string key.
    DATE          = 5,  // A text box that only allows dates. Stored as a timestamp
    ACCOUNT       = 6,  // A combobox that displays accounts from a certain provider
    LIST          = 7,  // Zero or more fields of the same type. Stored as a list of values
    STRUCT        = 8,  // A group of other fields. Stored as a list of key-value pairs
    ACTION        = 9,  // A button. Does not store any value, but triggers a callback when clicked
    FIELD_ACTION  = 10, // Another field. Does not store any value, but triggers a callback when edited
    CUSTOM        = 11, // Loads a custom qml document from the plugin folder. Stored as a list of key-value pairs
};

#define _NBL_FIELD_FLAG(type, v) (static_cast<u64>(v) << 16) << (static_cast<u8>(type) * 4)

// Affects how fields are saved, loaded, and displayed
enum class FieldFlag : u64 {

    HIDDEN = 0x1, // This field will not be visible in the UI
    SECRET = 0x2, // TODO: This field's value will be encrypted. The key should be placed in secret storage. Unimplemented because it breaks portability, and I'd need a way to let users import keys. (Just make them input a password? PBES2?)
    EPHEMERAL = 0x4, // Not stored
    
    STRICT_PLACEHOLDERS = _NBL_FIELD_FLAG(FieldType::STRING, 0x1), // Unmatched placeholders will cause an error to be thrown during serialization
    APPLY_GAME_PLACEHOLDERS = _NBL_FIELD_FLAG(FieldType::STRING, 0x2), // If this field belongs to an installation type, game-specific placeholders will be applied
    PASSWORD = _NBL_FIELD_FLAG(FieldType::STRING, 0x4), // The field should echo dots instead of characters

    SHOW_DEFAULT_OPTION = _NBL_FIELD_FLAG(FieldType::ACCOUNT, 0x1), // The "default" account option will be selectable

    HORIZONTAL = _NBL_FIELD_FLAG(FieldType::STRUCT, 0x1) // Struct field inputs will be placed left-to-right instead of top-to-bottom
};

#undef _NBL_FIELD_FLAG

typedef u64 FieldFlags;

inline FieldFlags operator|(FieldFlag flag, FieldFlag other) { return static_cast<u64>(flag) | static_cast<u8>(other); }
inline FieldFlags operator|(FieldFlag flag, FieldFlags other) { return static_cast<u64>(flag) | other; }
inline FieldFlags operator|(FieldFlags other, FieldFlag flag) { return static_cast<u64>(flag) | other; }

inline FieldFlags operator|=(FieldFlag& flag, FieldFlag other) { return static_cast<u64>(flag) | static_cast<u8>(other); }
inline FieldFlags operator|=(FieldFlag& flag, FieldFlags other) { return static_cast<u64>(flag) | other; }
inline FieldFlags operator|=(FieldFlags& other, FieldFlag flag) { return static_cast<u64>(flag) | other; }

inline FieldFlags operator&(FieldFlag flag, FieldFlag other) { return static_cast<u64>(flag) & static_cast<u8>(other); }
inline FieldFlags operator&(FieldFlag flag, FieldFlags other) { return static_cast<u64>(flag) & other; }
inline FieldFlags operator&(FieldFlags other, FieldFlag flag) { return static_cast<u64>(flag) & other; }

inline FieldFlags operator&=(FieldFlag& flag, FieldFlag other) { return static_cast<u64>(flag) | static_cast<u8>(other); }
inline FieldFlags operator&=(FieldFlag& flag, FieldFlags other) { return static_cast<u64>(flag) & other; }
inline FieldFlags operator&=(FieldFlags& other, FieldFlag flag) { return static_cast<u64>(flag) & other; }


struct FieldValue {
    union {
        struct {
            StringView resolved; 
            StringView raw; // No placeholders applied
            inline void set(StringView str) noexcept {
                resolved = str;
                raw = str;
            }
        } stringValue; // for STRING
        i64 integerValue; // for INTEGER
        u64 uintegerValue; // for DATE
        f64 floatValue; // for FLOAT
        bool booleanValue; // for BOOLEAN
        u32 enumValue; // for ENUM 
        struct {
            i64 id;
            bool isDefault;
        } accountValue; // for ACCOUNT
        Span<FieldValue> compositeValue; // for LIST, STRUCT, and CUSTOM
    };
};

class PluginContext;

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
            void (*pfnSuggest)(PluginContext* context, StringView current, std::function_ref<void(StringView)> suggestionConsumer, void* userData);
            void* userData;
            nbl::Span<nbl::Placeholder> customPlaceholders;

            void suggest(PluginContext* context, StringView current, std::function_ref<void(StringView)> suggestionConsumer) const {
                if(!pfnSuggest) return;
                pfnSuggest(context, current, suggestionConsumer, userData);
            }
        } stringData;
        struct {
            u32 (*pfnIndex)(PluginContext* context, StringView name, void* userData);
            StringView (*pfnName)(PluginContext* context, u32 index, void* userData);
            StringView (*pfnDisplayName)(PluginContext* context, u32 index, void* userData);
            u32 (*pfnCount)(PluginContext* context, void* userData);
            void* userData;

            u32 count(PluginContext* context) const { 
                if(!pfnCount) return 0;
                return pfnCount(context, userData); 
            }
            u32 index(PluginContext* context, StringView name) const { 
                if(!pfnIndex) return 0;
                return pfnIndex(context, name, userData); 
            }
            StringView name(PluginContext* context, u32 index) const { 
                if(!pfnName) return "";
                return pfnName(context, index, userData);
            }
            StringView displayName(PluginContext* context, u32 index) const { 
                if(!pfnDisplayName) return name(context, index);
                return pfnDisplayName(context, index, userData);
            }
        } enumData;
        struct {
            StringView accountProvider;
        } accountData;
        struct {
            Field* templateField;
            u32 min;
            u32 max;
        } listData;
        struct {
            Span<Field> fields;
        } structData;
        struct {
            void (*pfnActivate)(PluginContext* pluginContext, void* userData);
            void* userData;

            void activate(PluginContext* pluginContext) const {
                if(!pfnActivate) return;
                return pfnActivate(pluginContext, userData);
            }
        } actionData;
        struct {
            Field* templateField;
            void (*pfnActivate)(PluginContext* pluginContext, FieldValue value, void* userData);
            void* userData;

            void activate(PluginContext* pluginContext, FieldValue value) const {
                if(!pfnActivate) return;
                return pfnActivate(pluginContext, value, userData);
            }
        } fieldActionData;
        struct {
            StringView document;
        } customData;
    };
    FieldValue defaultValue;
    FieldFlags flags;
};

constexpr inline bool shouldEncodeField(Field field) {
    if(field.type == FieldType::ACTION || field.type == FieldType::FIELD_ACTION) return false;
    if(field.flags & FieldFlag::EPHEMERAL) return false;
    return true;
}

Result encodeFieldValues(PluginContext* context, Span<Field> fields, Span<FieldValue> values, Writer writer, Span<Placeholder> extraPlaceholders={});
Result decodeFieldValues(PluginContext* context, Span<Field> fields, JsonObject obj, Span<FieldValue>* outValues, LinearAllocator* allocator);

}
