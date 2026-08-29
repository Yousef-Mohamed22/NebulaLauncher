#include <Plugin/Field.hpp>
#include <Codec/Json.hpp>
#include <Platform/Allocator.hpp>


namespace {


nbl::Result _decodeFieldValue(const nbl::Field& field, nbl::JsonElement element, nbl::FieldValue* outValue, nbl::LinearAllocator* allocator) {

    switch(field.type) {
        case nbl::FieldType::STRING: 
            if(element.type != nbl::JsonType::STRING) return nbl::Result::INVALID_ARGUMENT;
            *outValue = nbl::FieldValue { .stringValue = element.string };
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
            u32 index = field.enumData.index(element.string);
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
            if(element.type != nbl::JsonType::ARRAY) return nbl::Result::INVALID_ARGUMENT;
            usize items = element.array.count;
            outValue->compositeValue.length = items;
            if(items > 0) {
                NBL_CHECK(allocator->allocate<nbl::FieldValue>(&outValue->compositeValue.data, items));
                for(usize i = 0 ; i < items ; i++) {
                    nbl::JsonElement ele = element.array[i];
                    NBL_CHECK(_decodeFieldValue(*field.listData.templateField, ele, &outValue->compositeValue[i], allocator));
                }
            }
            break;
        }
        case nbl::FieldType::STRUCT:
            if(element.type != nbl::JsonType::OBJECT) return nbl::Result::INVALID_ARGUMENT;
            NBL_CHECK(nbl::decodeFieldValues(field.structData.fields, element.object, &outValue->compositeValue, allocator));
            break;
    }
    
    return nbl::Result::SUCCESS;
}

nbl::Result _encodeFieldValue(const nbl::Field& field, nbl::FieldValue value, nbl::Writer writer) {
    switch(field.type) {
        case nbl::FieldType::STRING:
            return nbl::writeJson(writer, value.stringValue);
        case nbl::FieldType::FLOAT:
            return nbl::writeJson(writer, value.floatValue);
        case nbl::FieldType::INTEGER:
            return nbl::writeJson(writer, value.integerValue);
        case nbl::FieldType::BOOLEAN:
            return nbl::writeJson(writer, value.booleanValue);
        case nbl::FieldType::ENUM: {
            nbl::StringView name = field.enumData.name(value.enumValue);
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
                            *field.listData.templateField,
                            listValue,
                            w);
                    });
                }
            });
            break;
        }
        case nbl::FieldType::STRUCT: {
            return encodeFieldValues(field.structData.fields, value.compositeValue, writer);
        }
    }
    return nbl::Result::INVALID_ARGUMENT;

}

}


nbl::Result nbl::encodeFieldValues(Span<Field> fields, Span<FieldValue> values, Writer writer) {

    return JsonWriter(writer).writeObject([&](JsonWriter::ObjectWriter& obj) {
        for(usize i = 0 ; i < fields.length ; i++) {
            const Field& field = fields[i];
            obj.write(field.key, [&](Writer writer) {
                return _encodeFieldValue(field, values[i], writer);
            });
        }
    });   
}

nbl::Result nbl::decodeFieldValues(Span<Field> fields, JsonObject obj, Span<FieldValue>* outValues, LinearAllocator* allocator) {

    Span<FieldValue> values;
    NBL_CHECK(allocator->allocate<FieldValue>(&values.data, fields.length));
    values.length = fields.length;

    for(usize i = 0 ; i < fields.length ; i++) {

        const Field& field = fields[i];

        const JsonElement* val = obj.get(field.key);
        if(val == nullptr) {
            values[i] = field.defaultValue;
        } else {
            NBL_CHECK(_decodeFieldValue(field, *val, &values[i], allocator));
        }
    }

    *outValues = values;

    return nbl::Result::SUCCESS;
}
