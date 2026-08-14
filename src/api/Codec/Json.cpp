#include <Codec/Json.hpp>

const nbl::JsonElement* nbl::JsonObject::get(nbl::StringView key) const {

    if(bucketCount == 0) return nullptr;

    usize bucket = key.hash() % bucketCount;
    for(usize i = 0 ; i < buckets[bucket].count ; i++) {
        Entry& ent = entries[i + buckets[bucket].offset];
        if(ent.key == key) {
            return &ent.value;
        }
    }
    return nullptr;
}

nbl::JsonElement* nbl::JsonArray::get(usize index) {
    if(index >= count) return nullptr;
    return elements + index;
}

const nbl::JsonElement* nbl::JsonArray::get(usize index) const {
    if(index >= count) return nullptr;
    return elements + index;
}

nbl::JsonObject nbl::JsonArray::getObject(usize index) const {
    const JsonElement* element = get(index);
    if(element == nullptr || element->type != JsonType::OBJECT) return {};
    return element->object;
}

nbl::JsonObject nbl::JsonArray::getObject(usize index, JsonObject defaultValue) const {
    const JsonElement* element = get(index);
    if(element == nullptr || element->type != JsonType::OBJECT) return defaultValue;
    return element->object;
}

nbl::JsonArray nbl::JsonArray::getArray(usize index, JsonArray defaultValue) const {
    const JsonElement* element = get(index);
    if(element == nullptr || element->type != JsonType::ARRAY) return defaultValue;
    return element->array;
}

nbl::StringView nbl::JsonArray::getString(usize index, StringView defaultValue) const {
    const JsonElement* element = get(index);
    if(element == nullptr || element->type != JsonType::STRING) return defaultValue;
    return element->string;
}

nbl::JsonNumber nbl::JsonArray::getNumber(usize index, JsonNumber defaultValue) const { 
    const JsonElement* element = get(index);
    if(element == nullptr || element->type != JsonType::NUMBER) return defaultValue;
    return element->number;
}

bool nbl::JsonArray::getBoolean(usize index, bool defaultValue) const {
    const JsonElement* element = get(index);
    if(element == nullptr || element->type != JsonType::BOOLEAN) return defaultValue;
    return element->boolean;
}


nbl::JsonObject nbl::JsonObject::getObject(StringView key, JsonObject defaultValue) const {
    const JsonElement* element = get(key);
    if(element == nullptr || element->type != JsonType::OBJECT) return defaultValue;
    return element->object;
}

nbl::JsonArray nbl::JsonObject::getArray(StringView key, JsonArray defaultValue) const {
    const JsonElement* element = get(key);
    if(element == nullptr || element->type != JsonType::ARRAY) return defaultValue;
    return element->array;
}

nbl::StringView nbl::JsonObject::getString(StringView key, StringView defaultValue) const {
    const JsonElement* element = get(key);
    if(element == nullptr || element->type != JsonType::STRING) return defaultValue;
    return element->string;
}

nbl::JsonNumber nbl::JsonObject::getNumber(StringView key, JsonNumber defaultValue) const { 
    const JsonElement* element = get(key);
    if(element == nullptr || element->type != JsonType::NUMBER) return defaultValue;
    return element->number;
}

bool nbl::JsonObject::getBoolean(StringView key, bool defaultValue) const {
    const JsonElement* element = get(key);
    if(element == nullptr || element->type != JsonType::BOOLEAN) return defaultValue;
    return element->boolean;
}
