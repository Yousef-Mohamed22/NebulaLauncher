#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Assert.hpp>

#include <functional>
#include "Platform/Allocator.hpp"

namespace nbl {

enum class JsonType {
    NONE,
    NUMBER,
    BOOLEAN,
    STRING,
    ARRAY,
    OBJECT
};

enum class JsonNumberType {
    SIGNED,
    UNSIGNED,
    FLOAT
};

struct JsonNumber {
    JsonNumberType type;
    union {
        u64 uintValue;
        i64 intValue;
        f64 floatValue;
    };

    u64 asUint() const noexcept {
        switch(type) {
            case JsonNumberType::SIGNED: return static_cast<u64>(intValue);
            case JsonNumberType::UNSIGNED: return uintValue;
            case JsonNumberType::FLOAT: return static_cast<u64>(floatValue);
        }
        return 0;
    }

    i64 asInt() const noexcept {
        switch(type) {
            case JsonNumberType::SIGNED: return intValue;
            case JsonNumberType::UNSIGNED: return static_cast<i64>(intValue);
            case JsonNumberType::FLOAT: return static_cast<i64>(floatValue);
        }
        return 0;
    }

    f64 asFloat() const noexcept {
        switch(type) {
            case JsonNumberType::SIGNED: return static_cast<f64>(intValue);
            case JsonNumberType::UNSIGNED: return static_cast<f64>(uintValue);
            case JsonNumberType::FLOAT: return floatValue;
        }
        return 0.0;
    }
};

struct JsonObject;
struct JsonElement;
struct JsonArray {
    JsonElement* elements;
    usize count;

    JsonElement* get(usize index);
    const JsonElement* get(usize index) const;

    JsonElement& operator[](usize index);
    const JsonElement& operator[](usize index) const;

    inline JsonElement* begin() { return elements; }
    JsonElement* end();

    inline const JsonElement* begin() const { return elements; }
    const JsonElement* end() const;

    JsonObject getObject(usize index) const;
    JsonObject getObject(usize index, JsonObject defaultValue) const;
    JsonArray getArray(usize index, JsonArray defaultValue={}) const;
    StringView getString(usize index, StringView defaultValue={}) const;
    JsonNumber getNumber(usize index, JsonNumber defaultValue={}) const;
    bool getBoolean(usize index, bool defaultValue=false) const;
};

struct JsonObject {
    struct Entry;
    struct Bucket {
        usize offset;
        usize count;
    };

    Entry* entries;
    usize entryCount;

    Bucket* buckets;
    usize bucketCount;

    const JsonElement* get(StringView key) const;
    bool has(StringView key) const { return get(key) != nullptr; }

    inline Entry* begin() { return entries; }
    Entry* end();

    inline const Entry* begin() const { return entries; }
    const Entry* end() const;

    JsonObject getObject(StringView key, JsonObject defaultValue={}) const;
    JsonArray getArray(StringView key, JsonArray defaultValue={}) const;
    StringView getString(StringView key, StringView defaultValue={}) const;
    JsonNumber getNumber(StringView key, JsonNumber defaultValue={}) const;
    bool getBoolean(StringView key, bool defaultValue=false) const;
};

struct JsonElement {
    JsonType type;
    union {
        JsonNumber number;
        bool boolean;
        StringView string;
        JsonArray array;
        JsonObject object;
    };

    JsonElement() :
        type(JsonType::NONE)
    {} 

    JsonElement(u64 uint) :
        type(JsonType::NUMBER),
        number{ .type = JsonNumberType::UNSIGNED, .uintValue = uint }
    {}
    
    JsonElement(u32 uint) :
        type(JsonType::NUMBER),
        number{ .type = JsonNumberType::UNSIGNED, .uintValue = uint }
    {}

    JsonElement(i64 sint) :
        type(JsonType::NUMBER),
        number{ .type = JsonNumberType::SIGNED, .intValue = sint }
    {}

    JsonElement(i32 sint) :
        type(JsonType::NUMBER),
        number{ .type = JsonNumberType::SIGNED, .intValue = sint }
    {}

    JsonElement(f64 dbl) :
        type(JsonType::NUMBER),
        number{ .type = JsonNumberType::FLOAT, .floatValue = dbl }
    {}

    JsonElement(bool boolean) :
        type(JsonType::BOOLEAN),
        boolean(boolean)
    {}
    
    JsonElement(StringView string) :
        type(JsonType::STRING),
        string(string)
    {}
 
    JsonElement(const char* string) :
        type(JsonType::STRING),
        string(string)
    {}

    JsonElement(const JsonArray& array) :
        type(JsonType::ARRAY),
        array(array)
    {}

    JsonElement(const JsonObject& object) :
        type(JsonType::OBJECT),
        object(object)
    {}
};

struct JsonObject::Entry {
    StringView key;
    JsonElement value;
};


inline JsonElement& JsonArray::operator[](usize index) {
    NBL_ASSERT(index < count);
    return elements[index];
}

inline const JsonElement& JsonArray::operator[](usize index) const {
    NBL_ASSERT(index < count);
    return elements[index];
}

inline JsonElement* JsonArray::end() {
    return elements + count;
}
inline const JsonElement* JsonArray::end() const {
    return elements + count;
}

inline JsonObject::Entry* JsonObject::end() {
    return entries + entryCount;
}
inline const JsonObject::Entry* JsonObject::end() const {
    return entries + entryCount;
}

Result parseJson(Reader reader, LinearAllocator* storage, JsonElement* outElement, LinearAllocator* tempStorage = nullptr);
Result parseJsonNumber(Reader reader, JsonNumber* outNumber);
Result writeJson(Writer writer, JsonElement element);

inline Result toString(JsonElement element, Writer writer) { return writeJson(writer, element); }
inline Result toString(JsonArray element, Writer writer) { return writeJson(writer, element); }
inline Result toString(JsonObject element, Writer writer) { return writeJson(writer, element); }

class JsonWriter {
public:

    JsonWriter(Writer writer) :
        _mLastResult(nbl::Result::SUCCESS),
        _mSink(writer)
    { }

    class ObjectWriter;

    class ArrayWriter {
    public:
        ArrayWriter& add(JsonElement element);
        ArrayWriter& writeObject(std::function_ref<void(ObjectWriter& writer)> writer);
        ArrayWriter& writeArray(std::function_ref<void(ArrayWriter& writer)> writer);
    private:

        ArrayWriter(JsonWriter& parent) :
            _rParent(parent)
        { }

        usize _mNumElements = 0;
        JsonWriter& _rParent;
        friend class JsonWriter;
    };

    class ObjectWriter {
    public:
        ObjectWriter& add(StringView key, JsonElement element);
        ObjectWriter& writeObject(StringView key, std::function_ref<void(ObjectWriter& writer)> writer);
        ObjectWriter& writeArray(StringView key, std::function_ref<void(ArrayWriter& writer)> writer);
    private:

        ObjectWriter(JsonWriter& parent) :
            _rParent(parent)
        { }

        usize _mNumElements = 0;
        JsonWriter& _rParent;
        friend class JsonWriter;
    };


    Result writeElement(JsonElement element);
    Result writeObject(std::function_ref<void(ObjectWriter& writer)> writer);
    Result writeArray(std::function_ref<void(ArrayWriter& writer)> writer);

private:

    Result _mLastResult;
    Writer _mSink;

};


}
