#include <Plugin/Field.hpp>
#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>
#include "Core/Span.hpp"
#include "Utility/Placeholder.hpp"


namespace {


nbl::Result _decodeFieldValue(nbl::PluginContext* context, const nbl::Field& field, nbl::JsonElement element, nbl::FieldValue* outValue, nbl::LinearAllocator* allocator) {

    switch(field.type) {
        case nbl::FieldType::STRING: 
            if(element.type == nbl::JsonType::STRING) {
                *outValue = nbl::FieldValue { 
                    .stringValue = {
                        .resolved = element.string,
                        .raw = element.string,
                    }
                };
            } else if(element.type == nbl::JsonType::OBJECT) {
                *outValue = nbl::FieldValue {
                    .stringValue = {
                        .resolved = element.object.getString("resolved"),
                        .raw = element.object.getString("raw")
                    }
                };
            } else {
                return nbl::Result::INVALID_ARGUMENT;
            }
            break;
        case nbl::FieldType::FLOAT:
            if(element.type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
            *outValue = nbl::FieldValue { .floatValue = element.number.asFloat() };
            break;
        case nbl::FieldType::INTEGER:
            if(element.type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
            *outValue = nbl::FieldValue { .integerValue = element.number.asInt() };
            break;
        case nbl::FieldType::BOOLEAN:
            if(element.type != nbl::JsonType::BOOLEAN) return nbl::Result::INVALID_ARGUMENT;
            *outValue = nbl::FieldValue { .booleanValue = element.boolean };
            break;
        case nbl::FieldType::ENUM: {
            if(element.type != nbl::JsonType::STRING) return nbl::Result::INVALID_ARGUMENT;
            u32 index = field.enumData.index(context, element.string);
            *outValue = nbl::FieldValue { .enumValue = index };
            break;
        }
        case nbl::FieldType::DATE: 
            if(element.type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
            *outValue = nbl::FieldValue { .uintegerValue = element.number.asUint() };
            break;
        case nbl::FieldType::ACCOUNT:
            if(element.type != nbl::JsonType::NUMBER) return nbl::Result::INVALID_ARGUMENT;
            outValue->accountValue = {
                .id = static_cast<u32>(element.number.asUint()),
                .isDefault = (element.number.asInt() == -1)
            };
            break;
        case nbl::FieldType::LIST: {
            if(!nbl::shouldEncodeField(*field.listData.templateField)) {
                return nbl::Result::INVALID_ARGUMENT;
            }
            if(element.type != nbl::JsonType::ARRAY) return nbl::Result::INVALID_ARGUMENT;
            usize items = element.array.count;
            outValue->compositeValue.length = items;
            if(items > 0) {
                NBL_CHECK(allocator->allocate<nbl::FieldValue>(&outValue->compositeValue.data, items));
                for(usize i = 0 ; i < items ; i++) {
                    nbl::JsonElement ele = element.array[i];
                    NBL_CHECK(_decodeFieldValue(context, *field.listData.templateField, ele, &outValue->compositeValue[i], allocator));
                }
            }
            break;
        }
        case nbl::FieldType::STRUCT:
        case nbl::FieldType::CUSTOM:
            if(element.type != nbl::JsonType::OBJECT) return nbl::Result::INVALID_ARGUMENT;
            NBL_CHECK(nbl::decodeFieldValues(context, field.structData.fields, element.object, &outValue->compositeValue, allocator));
            break;
        case nbl::FieldType::ACTION:
        case nbl::FieldType::FIELD_ACTION:
            return nbl::Result::INVALID_ARGUMENT;
    }
    
    return nbl::Result::SUCCESS;
}

nbl::Result _encodeFieldValue(nbl::PluginContext* context, const nbl::Field& field, nbl::FieldValue value, nbl::Writer writer, nbl::Span<nbl::Placeholder> extraPlaceholders) {
    switch(field.type) {
        case nbl::FieldType::STRING: {

            if(value.stringValue.raw == value.stringValue.resolved) {
                NBL_CHECK(writer.writeChar('"'));
                nbl::Writer w = nbl::jsonStringWriter(&writer);
                NBL_CHECK(w.writeString(value.stringValue.raw));
                return writer.writeChar('"');
            } else {
                return nbl::JsonWriter(writer).writeObject([&](auto obj) {
                    obj.add("raw", value.stringValue.raw);
                    obj.writeString("resolved", [&](auto writer){
                        return nbl::formatPlaceholders(value.stringValue.raw, writer, nbl::Array { field.stringData.customPlaceholders, extraPlaceholders }, (field.flags & nbl::FieldFlag::STRICT_PLACEHOLDERS));
                    });
                });
            }
        }
        case nbl::FieldType::FLOAT:
            return nbl::writeJson(writer, value.floatValue);
        case nbl::FieldType::INTEGER:
            return nbl::writeJson(writer, value.integerValue);
        case nbl::FieldType::BOOLEAN:
            return nbl::writeJson(writer, value.booleanValue);
        case nbl::FieldType::ENUM: {
            nbl::StringView name = field.enumData.name(context, value.enumValue);
            return nbl::writeJson(writer, name);
        }
        case nbl::FieldType::DATE: 
            return nbl::writeJson(writer, value.uintegerValue);
        case nbl::FieldType::ACCOUNT: {
            if(value.accountValue.isDefault) {
                return writer.writeString("-1");
            } else {
                return nbl::writeJson(writer, value.accountValue.id);
            }
            break;
        }
        case nbl::FieldType::LIST: {
            return nbl::JsonWriter(writer).writeArray([&](auto& arr) {
                for(nbl::FieldValue& listValue : value.compositeValue) {
                    arr.write([&](nbl::Writer w) {
                        return _encodeFieldValue(
                            context,
                            *field.listData.templateField,
                            listValue,
                            w,
                            extraPlaceholders);
                    });
                }
            });
            break;
        }
        case nbl::FieldType::STRUCT: 
        case nbl::FieldType::CUSTOM: {
            return encodeFieldValues(context, field.structData.fields, value.compositeValue, writer);
        }
        case nbl::FieldType::ACTION:
        case nbl::FieldType::FIELD_ACTION:
            return nbl::Result::INVALID_ARGUMENT;
    }
    return nbl::Result::INVALID_ARGUMENT;

}

}


nbl::Result nbl::encodeFieldValues(PluginContext* context, Span<Field> fields, Span<FieldValue> values, Writer writer, Span<Placeholder> extraPlaceholders) {

    u8 buffer[4096]{};
    BufferedWriter bufferedWriter(writer, buffer, 4096);

    return JsonWriter(bufferedWriter).writeObject([&](JsonWriter::ObjectWriter& obj) {
        for(usize i = 0 ; i < fields.length ; i++) {
            const Field& field = fields[i];

            if(!shouldEncodeField(field)) continue;
            else if(field.type == FieldType::LIST && !shouldEncodeField(*field.listData.templateField)) continue;

            obj.write(field.key, [&](Writer writer) {
                return _encodeFieldValue(context, field, values[i], writer, extraPlaceholders);
            });
        }
    });   
}


nbl::Result nbl::decodeFieldValues(PluginContext* context, Span<Field> fields, JsonObject obj, Span<FieldValue>* outValues, LinearAllocator* allocator) {
    Span<FieldValue> values;
    NBL_CHECK(allocator->allocate<FieldValue>(&values.data, fields.length));
    values.length = fields.length;

    for(usize i = 0 ; i < fields.length ; i++) {

        const Field& field = fields[i];

        if(!shouldEncodeField(field)) continue;
        else if(field.type == FieldType::LIST && !shouldEncodeField(*field.listData.templateField)) continue;

        const JsonElement* val = obj.get(field.key);
        if(val == nullptr) {
            values[i] = field.defaultValue;
        } else {
            NBL_CHECK(_decodeFieldValue(context, field, *val, &values[i], allocator));
        }
    }

    *outValues = values;

    return nbl::Result::SUCCESS;
}
