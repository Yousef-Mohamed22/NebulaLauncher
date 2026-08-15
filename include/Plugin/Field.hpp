#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <functional>

namespace nbl {

enum FieldType {
    STRING = 0,
    INTEGER = 1,
    FLOAT = 2,
    BOOLEAN = 3,
    ENUM = 4,
    DATE = 5,
    ACCOUNT = 6,
    LIST = 7,
    STRUCT = 8
};

struct FieldValue {
    union {
        StringView stringValue; // for STRING
        i64 integerValue; // for INTEGER
        u64 uintegerValue; // for DATE
        f64 floatValue; // for FLOAT
        bool booleanValue; // for BOOLEAN
        u32 enumValue; // for ENUM and ACCOUNT
        Span<FieldValue> compositeValue; // for LIST and STRUCT
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
            void (*pfnSuggest)(StringView current, std::function_ref<void(StringView)> suggestionConsumer, void* userData);
            void* userData;

            void suggest(StringView current, std::function_ref<void(StringView)> suggestionConsumer) const {
                if(pfnSuggest) {
                    pfnSuggest(current, suggestionConsumer, userData);
                }
            }
        } stringData;
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
        struct {
            StringView accountProvider;
            bool showDefaultOption;
        } accountData;
        struct {
            Field* templateField;
            u32 min;
            u32 max;
        } listData;
        struct {
            Span<Field> fields;
        } structData;
    };
    FieldValue defaultValue;
};


}
