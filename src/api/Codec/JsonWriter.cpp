// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file JsonWriter.cpp
 * \author jwall64
 * \brief Implementation of JSON encoder.
 */

#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Codec/Json.hpp>

namespace {

nbl::Result _writeBool(nbl::Writer writer, bool value) {
    if(value) {
        return writer.writeString("true");
    } else {
        return writer.writeString("false");
    }
}

nbl::Result _writeNumber(nbl::Writer writer, nbl::JsonNumber number) {
    switch(number.type) {
        case nbl::JsonNumberType::FLOAT:
            return nbl::toString(number.asFloat(), writer);
        case nbl::JsonNumberType::SIGNED:
            return nbl::toString(number.asInt(), writer);
        case nbl::JsonNumberType::UNSIGNED:
            return nbl::toString(number.asUint(), writer);
    }
    return nbl::error::UNEXPECTED_ERROR;
}

nbl::Result _writeString(nbl::Writer writer, const nbl::StringView& str) {
    NBL_CHECK(writer.writeChar('"'));
    NBL_CHECK(writer.writeString(str));
    return writer.writeChar('"');
}

nbl::Result _writeNull(nbl::Writer writer) {
    return writer.writeString("null");
}


}

nbl::Result nbl::writeJson(nbl::Writer writer, const nbl::JsonValue& value) {
    switch(value.type) {
        case nbl::JsonType::BOOLEAN:
            return _writeBool(writer, value.boolean);
        case nbl::JsonType::NUMBER:
            return _writeNumber(writer, value.number);
        case nbl::JsonType::STRING:
            return _writeString(writer, value.string);
        case nbl::JsonType::ARRAY:
            return writeJsonArray(writer, value.array);
        case nbl::JsonType::OBJECT:
            return writeJsonObject(writer, value.object);
        case nbl::JsonType::NONE:
            return _writeNull(writer);
        default:
            nbl::log(nbl::LogLevel::ERROR, "Invalid JSON value type: ", static_cast<i32>(value.type));
            return nbl::error::UNEXPECTED_ERROR;
    }
}

nbl::Result nbl::writeJsonArray(nbl::Writer writer, const nbl::JsonArray& array) {
    NBL_CHECK(writer.writeChar('['));
    usize i = 0;
    for(const nbl::JsonValue& value : array) {
        if(i++ > 0) {
            NBL_CHECK(writer.writeChar(','));
        }
        NBL_CHECK(nbl::writeJson(writer, value));
    }
    return writer.writeChar(']');
}

nbl::Result nbl::writeJsonObject(nbl::Writer writer, const nbl::JsonObject& object) {
    NBL_CHECK(writer.writeChar('{'));
    size_t count = 0;
    for(const auto& [key, value] : object) {
        if(count > 0) {
            NBL_CHECK(writer.writeChar(','));
        }
        NBL_CHECK(_writeString(writer, key));
        NBL_CHECK(writer.writeChar(':'));
        NBL_CHECK(nbl::writeJson(writer, value));
        count++;
    }
    return writer.writeChar('}');
}
