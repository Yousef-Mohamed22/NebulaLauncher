// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file JsonParser.cpp
 * \author jwall64
 * \brief Implementation of JSON parser.
 */

#include <Core/Result.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Logging.hpp>
#include <Codec/Json.hpp>
#include <Codec/Utf8.hpp>

#ifndef NBL_IN_PLACE_ARRAY_FRAME_SIZE
#define NBL_IN_PLACE_ARRAY_FRAME_SIZE 32
#endif

#ifndef NBL_IN_PLACE_OBJECT_FRAME_SIZE
#define NBL_IN_PLACE_OBJECT_FRAME_SIZE 32
#endif

namespace {

struct _ParseContext {
    nbl::Reader reader;
    nbl::ArenaFrame* storage;
    nbl::ArenaFrame* temporaryStorage;
    nbl::JsonValue* outValue;
    char lastReadChar;
};

nbl::Result _parseValue(_ParseContext* context);

nbl::Result _nextToken(nbl::Reader reader, char* lastReadChar) {
    while(true) {
        char c;
        nbl::Result res = reader.readAll(&c, 1);
        if(res == nbl::error::UNEXPECTED_EOF) {
            *lastReadChar = 0;
            return nbl::Result::SUCCESS;
        }
        NBL_CHECK(res);

        *lastReadChar = c;
        if(c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            break;
        }
    }
    return nbl::Result::SUCCESS;
}

nbl::Result _nextToken(_ParseContext* context) {
    return _nextToken(context->reader, &context->lastReadChar);
}

nbl::Result _readString(nbl::Reader reader, nbl::StringBuilder* builder, char* lastReadChar) {

    // Assume lastReadChar is '"'

    bool escaped = false;

    while(true) {
        char c;
        NBL_CHECK(reader.readAll(&c, 1));
        *lastReadChar = c;

        if(escaped) {
            escaped = false;
            switch(c) {
                case '"':
                case '\\':
                case '/':
                    if(builder) builder->append(c);
                    break;
                case 'b':
                    if(builder) builder->append('\b');
                    break;
                case 'f':
                    if(builder) builder->append('\f');
                    break;
                case 'n':
                    if(builder) builder->append('\n');
                    break;
                case 'r':
                    if(builder) builder->append('\r');
                    break;
                case 't':
                    if(builder) builder->append('\t');
                    break;
                case 'u': {
                    char hex[4];
                    NBL_CHECK(reader.readAll(hex, 4));
                    uint16_t codePoint = 0;
                    for(int i = 0; i < 4; i++) {
                        char h = hex[i];
                        codePoint <<= 4;
                        if(h >= '0' && h <= '9') {
                            codePoint |= (h - '0');
                        } else if(h >= 'a' && h <= 'f') {
                            codePoint |= (h - 'a' + 10);
                        } else if(h >= 'A' && h <= 'F') {
                            codePoint |= (h - 'A' + 10);
                        } else {
                            return nbl::error::UNEXPECTED_TOKEN;
                        }
                    }
                    nbl::writeCodePoint(*builder, codePoint);
                }
                default:
                    return nbl::error::UNEXPECTED_TOKEN;
            }
        } else {
            if(c == '"') {
                break;
            }
            if(c == '\\') {
                escaped = true;
            } else {
                if(builder) builder->append(c);
            }
        }
    }
    return _nextToken(reader, lastReadChar);
}

nbl::Result _parseString(_ParseContext* context) {
    if(context->storage == nullptr) {

        NBL_CHECK(_readString(context->reader, nullptr, &context->lastReadChar));

        context->outValue->string = {};
        context->outValue->type = nbl::JsonType::STRING;
        return nbl::Result::SUCCESS;
    }

    nbl::StringBuilder builder(context->storage);
    NBL_CHECK(_readString(context->reader, &builder, &context->lastReadChar));

    builder.buildNullTerminated(&context->outValue->string);
    usize length = context->outValue->string.length + 1;

    usize alignedLength = nbl::align(length + 1, sizeof(void*));
    if(alignedLength > length + 1) {
        void* paddingBytes;
        NBL_CHECK(context->storage->push(alignedLength - (length + 1), &paddingBytes));
        memset(paddingBytes, 0, alignedLength - (length + 1));
    }
    
    context->outValue->type = nbl::JsonType::STRING;
    return nbl::Result::SUCCESS;
}

nbl::Result _skipArray(_ParseContext* context) {

    usize count = 0;
    do {
        if(count > 0) {
            NBL_CHECK(_nextToken(context));
        }
        NBL_CHECK(_parseValue(context));
        count++;
    } while(context->lastReadChar == ',');

    if(context->lastReadChar != ']') {
        return nbl::error::UNEXPECTED_TOKEN;
    }

    return _nextToken(context);
}

nbl::Result _parseArrayInPlace(_ParseContext* context, usize depth = 0) {

    nbl::JsonValue values[NBL_IN_PLACE_ARRAY_FRAME_SIZE];
    usize index = 0;

    do {
        if(index > 0 || depth > 0) {
            NBL_CHECK(_nextToken(context));
        }
    
        NBL_CHECK(_parseValue(context));
        values[index++] = *context->outValue;

    } while(index < NBL_IN_PLACE_ARRAY_FRAME_SIZE && context->lastReadChar == ',');

    usize arrayLength = 0;
    usize offset = NBL_IN_PLACE_ARRAY_FRAME_SIZE * depth;
    nbl::JsonValue* arrayStart = nullptr;

    // Abuse recursion to use the stack for temporary storage.
    if(context->lastReadChar == ',') {

        _parseArrayInPlace(context, depth + 1);
        arrayLength = context->outValue->array.length();
        arrayStart = const_cast<nbl::JsonValue*>(context->outValue->array.data());

    } else if(context->lastReadChar != ']') {

        return nbl::error::UNEXPECTED_TOKEN;

    } else {

        // Allocate enough space for all the entries
        arrayLength = offset + index;
        context->storage->allocate(&arrayStart, arrayLength);
    }

    memcpy(arrayStart + offset, values, sizeof(nbl::JsonValue) * index);

    context->outValue->type = nbl::JsonType::ARRAY;
    context->outValue->array = nbl::JsonArray(arrayStart, arrayLength);

    if(depth == 0) {
        return _nextToken(context);
    } else {
        return nbl::Result::SUCCESS;
    }
}

nbl::Result _parseArray(_ParseContext* context) {

    // Assume lastReadChar is '['
    NBL_CHECK(_nextToken(context));
    if(context->lastReadChar == ']') {
        // Empty array
        context->outValue->type = nbl::JsonType::ARRAY;
        context->outValue->array = nbl::JsonArray::EMPTY;
        return _nextToken(context);
    }

    if(context->storage == nullptr) {
        return _skipArray(context);
    }

    if(context->temporaryStorage == nullptr) {
        return _parseArrayInPlace(context);
    }

    nbl::JsonValue* values;
    usize count = 0;
    {
        nbl::ArenaFrame tempFrame(context->temporaryStorage);
        nbl::JsonValue* tempValues = static_cast<nbl::JsonValue*>(tempFrame.current());
        do {
            if(count > 0) {
                NBL_CHECK(_nextToken(context));
            }
            NBL_CHECK(_parseValue(context));
            NBL_CHECK(tempFrame.pushValue(*context->outValue));
            count++;
        } while(context->lastReadChar == ',');

        if(context->lastReadChar != ']') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(context->storage->allocate<nbl::JsonValue>(&values, count));
        memcpy(values, tempValues, sizeof(nbl::JsonValue) * count);
    }

    context->outValue->type = nbl::JsonType::ARRAY;
    context->outValue->array = nbl::JsonArray(values, count);

    return _nextToken(context);
}

nbl::Result _skipObject(_ParseContext* context) {

    do {
        if(context->lastReadChar != '"') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(_parseString(context));

        if(context->lastReadChar != ':') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(_nextToken(context));
        NBL_CHECK(_parseValue(context));

    } while(context->lastReadChar == ',');

    if(context->lastReadChar != '}') {
        return nbl::error::UNEXPECTED_TOKEN;
    }

    return _nextToken(context);
}

nbl::Result _parseObjectInPlace(_ParseContext* context, usize depth = 0) {

    nbl::JsonObject::Entry entries[NBL_IN_PLACE_OBJECT_FRAME_SIZE];
    usize index = 0;

    do {
        if(index > 0 || depth > 0) {
            NBL_CHECK(_nextToken(context));
        }
        if(context->lastReadChar != '"') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        nbl::StringView key;
        NBL_CHECK(_parseString(context));
        key = context->outValue->string;

        if(context->lastReadChar != ':') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(_nextToken(context));
        NBL_CHECK(_parseValue(context));
        entries[index++] = { key, *context->outValue };

    } while(index < NBL_IN_PLACE_OBJECT_FRAME_SIZE && context->lastReadChar == ',');

    usize objectSize = 0;
    nbl::JsonObject::Entry* objectStart = nullptr;

    // Abuse recursion to use the stack for temporary storage.
    if(context->lastReadChar == ',') {
        _parseObjectInPlace(context, depth + 1);
        
        // Disgusting hack
        objectStart = const_cast<nbl::JsonObject::Entry*>(reinterpret_cast<const nbl::JsonObject::Entry*>(context->outValue->array.data()));
        objectSize = NBL_IN_PLACE_OBJECT_FRAME_SIZE + context->outValue->array.length();

    } else if(context->lastReadChar != '}') {

        return nbl::error::UNEXPECTED_TOKEN;

    } else {

        objectSize = index;
    }

    // Push the values onto permanent storage
    nbl::JsonObject::Entry* storedEntries;

    context->storage->allocate(&storedEntries, index);
    if(objectStart == nullptr) objectStart = storedEntries;

    memcpy(storedEntries, entries, sizeof(nbl::JsonObject::Entry) * index);

    if(depth > 0) {

        // Hack
        context->outValue->type = nbl::JsonType::NONE;
        context->outValue->array = nbl::JsonArray(reinterpret_cast<nbl::JsonValue*>(objectStart), objectSize);
        return nbl::Result::SUCCESS;

    } else {

        NBL_CHECK(nbl::JsonObject::buildFromUnorderedEntries({ objectStart, objectSize }, context->storage, &context->outValue->object));
        context->outValue->type = nbl::JsonType::OBJECT;
        return _nextToken(context);
    }
}

nbl::Result _parseObject(_ParseContext* context) {
   
    // Assume lastReadChar is '{'
    NBL_CHECK(_nextToken(context)); 
    if(context->lastReadChar == '}') {
        // Empty object
        context->outValue->type = nbl::JsonType::OBJECT;
        context->outValue->object = nbl::JsonObject::EMPTY;
        return _nextToken(context);
    }

    if(context->storage == nullptr) {
        return _skipObject(context);
    }

    if(context->temporaryStorage == nullptr) {
        return _parseObjectInPlace(context);
    }


    nbl::JsonObject::Entry* entries;
    usize count = 0;
    
    {
        nbl::ArenaFrame tempFrame(context->temporaryStorage);
        nbl::JsonObject::Entry* tempEntries = static_cast<nbl::JsonObject::Entry*>(tempFrame.current());

        do {
            if(count > 0) {
                NBL_CHECK(_nextToken(context));
            }
            if(context->lastReadChar != '"') {
                return nbl::error::UNEXPECTED_TOKEN;
            }

            nbl::StringView key;
            NBL_CHECK(_parseString(context));
            key = context->outValue->string;

            if(context->lastReadChar != ':') {
                return nbl::error::UNEXPECTED_TOKEN;
            }

            NBL_CHECK(_nextToken(context));
            NBL_CHECK(_parseValue(context));
        
            NBL_CHECK(tempFrame.pushValue(nbl::JsonObject::Entry { key, *context->outValue }));
            count++;

        } while(context->lastReadChar == ',');

        if(context->lastReadChar != '}') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(context->storage->allocate(&entries, count));
        memcpy(entries, tempEntries, sizeof(nbl::JsonObject::Entry) * count);
    }

    context->outValue->type = nbl::JsonType::OBJECT;
    NBL_CHECK(nbl::JsonObject::buildFromUnorderedEntries({ entries, count }, context->storage, &context->outValue->object));

    return _nextToken(context);
}


template<typename T>
T _applyExponent(T value, u64 exponent, bool negative) {
    
    if(exponent == 0) {
        return value;
    }

    T result = 1;
    for(u64 i = 0; i < exponent; i++) {
        result *= 10;
    }
    return value * (negative ? (1 / result) : result);
}


nbl::Result _parseNumber(_ParseContext* context) {

    nbl::JsonNumberType numberType = nbl::JsonNumberType::UNSIGNED;

    bool negative = false;
    u64 wholePart = 0;
    f64 fractionPart = 0;

    if(context->lastReadChar == '-') {
        // Negative number
        negative = true;
        numberType = nbl::JsonNumberType::SIGNED;
        NBL_CHECK(_nextToken(context));
    } 

    if(context->lastReadChar == '0') {
        // Leading zero is only allowed if it's the only digit
        NBL_CHECK(_nextToken(context));
    } else if(context->lastReadChar >= '1' && context->lastReadChar <= '9') {
        // Parse whole part
        do {
            wholePart = wholePart * 10 + (context->lastReadChar - '0');
            NBL_CHECK(_nextToken(context));
        } while(context->lastReadChar >= '0' && context->lastReadChar <= '9');
    } else {
        return nbl::error::UNEXPECTED_TOKEN;
    }

    if(context->lastReadChar == '.') {
        // Parse fraction part
        numberType = nbl::JsonNumberType::FLOAT;
        NBL_CHECK(_nextToken(context));
        if(context->lastReadChar < '0' || context->lastReadChar > '9') {
            return nbl::error::UNEXPECTED_TOKEN;
        }

        f64 divisor = 10;
        do {
            fractionPart += (context->lastReadChar - '0') / divisor;
            divisor *= 10;
            NBL_CHECK(_nextToken(context));
        } while(context->lastReadChar >= '0' && context->lastReadChar <= '9');
    }


    u64 exponent = 0;
    bool exponentNegative = false;
    if(context->lastReadChar == 'e' || context->lastReadChar == 'E') {
        // Parse exponent part
        NBL_CHECK(_nextToken(context));
        if(context->lastReadChar == '-') {
            exponentNegative = true;
            numberType = nbl::JsonNumberType::FLOAT;
            NBL_CHECK(_nextToken(context));
        } else if(context->lastReadChar == '+') {
            NBL_CHECK(_nextToken(context));
        }

        if(context->lastReadChar < '0' || context->lastReadChar > '9') {
            return nbl::error::UNEXPECTED_TOKEN;
        }
        do {
            exponent = exponent * 10 + (context->lastReadChar - '0');
            NBL_CHECK(_nextToken(context));
        } while(context->lastReadChar >= '0' && context->lastReadChar <= '9');
    }

    context->outValue->type = nbl::JsonType::NUMBER;
    if(numberType == nbl::JsonNumberType::FLOAT) {
        f64 value = wholePart + fractionPart;
        if(negative) {
            value = -value;
        }
        context->outValue->number.type = nbl::JsonNumberType::FLOAT;
        context->outValue->number.real = _applyExponent(value, exponent, exponentNegative);
    } else {
        if(negative) {
            context->outValue->number.sint = _applyExponent(-static_cast<i64>(wholePart), exponent, false);
            context->outValue->number.type = nbl::JsonNumberType::SIGNED;
        } else {
            context->outValue->number.uint = _applyExponent(wholePart, exponent, false);
            context->outValue->number.type = nbl::JsonNumberType::UNSIGNED;
        }
    }

    return nbl::Result::SUCCESS;
}

nbl::Result _parseValue(_ParseContext* context) {

    switch(context->lastReadChar) {
        case 'n': { // null
            char buffer[3];
            NBL_CHECK(context->reader.readAll(buffer, 3));
            if(buffer[0] == 'u' && buffer[1] == 'l' && buffer[2] == 'l') {
                *context->outValue = nbl::JsonValue::NULL_VALUE;
                NBL_CHECK(_nextToken(context));
                return nbl::Result::SUCCESS;
            }
            return nbl::error::UNEXPECTED_TOKEN;
        }
        case 't': { // true
            char buffer[3];
            NBL_CHECK(context->reader.readAll(buffer, 3));
            if(buffer[0] == 'r' && buffer[1] == 'u' && buffer[2] == 'e') {
                *context->outValue = nbl::JsonValue::TRUE_VALUE;
                NBL_CHECK(_nextToken(context));
                return nbl::Result::SUCCESS;
            }
            return nbl::error::UNEXPECTED_TOKEN;
        }   
        case 'f': { // false
            char buffer[4];
            NBL_CHECK(context->reader.readAll(buffer, 4));
            if(buffer[0] == 'a' && buffer[1] == 'l' && buffer[2] == 's' && buffer[3] == 'e') {
                *context->outValue = nbl::JsonValue::FALSE_VALUE;
                NBL_CHECK(_nextToken(context));
                return nbl::Result::SUCCESS;
            }
            return nbl::error::UNEXPECTED_TOKEN;
        }   
        case '"': // string
            return _parseString(context);
        case '[': // array
            return _parseArray(context);
        case '{': // object
            return _parseObject(context);
        default: // number or invalid token
            if((context->lastReadChar >= '0' && context->lastReadChar <= '9') || context->lastReadChar == '-') {
                return _parseNumber(context);
            }
            return nbl::error::UNEXPECTED_TOKEN;
    }
}

}



nbl::Result nbl::parseJson(Reader reader, ArenaFrame* storage, JsonValue* outValue, bool inPlace) {

    auto frame = inPlace ? ArenaFrame() : ArenaFrame(nbl::Arena::threadLocal());
    _ParseContext context {
        .reader = reader,
        .storage = storage,
        .temporaryStorage = inPlace ? nullptr : &frame,
        .outValue = outValue,
        .lastReadChar = 0
    };

    if(!inPlace && context.storage->arena() == context.temporaryStorage->arena()) {
        return nbl::error::ILLEGAL_ARGUMENT;
    }

    // Find the next token
    NBL_CHECK(_nextToken(&context));
 
    return _parseValue(&context);
}


nbl::Result nbl::parseJsonNumber(Reader reader, JsonNumber* outNumber) {

    JsonValue value;
    _ParseContext context {
        .reader = reader,
        .storage = nullptr,
        .temporaryStorage = {},
        .outValue = &value,
        .lastReadChar = 0
    };

    NBL_CHECK(_nextToken(&context));
    NBL_CHECK(_parseNumber(&context));
    if(value.type != JsonType::NUMBER) {
        return nbl::error::ILLEGAL_ARGUMENT;
    }
    
    *outNumber = value.number;
    return nbl::Result::SUCCESS;
}




nbl::PartialJsonParser& nbl::PartialJsonParser::beginObject() {
    if(_mLastResult.error()) {
        return *this;
    }
    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    if(_mLastResult.error()) {
        return *this;
    } else if(_mCurrentChar != '{') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        return *this;
    }

    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::endObject() {

    if(_mLastResult.error()) {
        return *this;
    }
    if(_mCurrentChar != '}') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        return *this;
    }
    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::readKey(nbl::StringBuilder* outKey) {
    if(_mLastResult.error()) {
        return *this;
    }

    if(_mCurrentChar == '{' || _mCurrentChar == ',') {
        nbl::Result res = _nextToken(_mReader, &_mCurrentChar);
        if(res.error()) {
            _mLastResult = res;
        } else if(_mCurrentChar != '"') {
            _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        } else {
            res = _readString(_mReader, outKey, &_mCurrentChar);
            if(res.error()) {
                _mLastResult = res;
            } else if(_mCurrentChar != ':') {
                _mLastResult = nbl::error::UNEXPECTED_TOKEN;
            }
        }
    } else {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
    }

    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::expectKey(StringView expectedKey) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    readKey(&builder);
    if(_mLastResult.error()) return *this;

    StringView key;
    _mLastResult = builder.build(&key);
    if(_mLastResult.error()) return *this;

    if(key != expectedKey) _mLastResult = nbl::error::ILLEGAL_STATE;
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::readValue(nbl::ArenaFrame* frame, nbl::JsonValue* outValue) {

    if(_mLastResult.error()) {
        return *this;
    }

    _ParseContext context {
        .reader = _mReader,
        .storage = frame,
        .temporaryStorage = nullptr,
        .outValue = outValue,
        .lastReadChar = _mCurrentChar
    };

    _mLastResult = _nextToken(&context);
    if(_mLastResult.error()) {
        return *this;
    }

    _mLastResult = _parseValue(&context);

    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::readString(nbl::StringBuilder* outString) {

    if(_mLastResult.error()) {
        return *this;
    }

    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    if(_mLastResult.error()) {
        return *this;
    }

    if(_mCurrentChar != '"') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        return *this;
    }

    _mLastResult = _readString(_mReader, outString, &_mCurrentChar);
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::readNumber(nbl::JsonNumber* outNumber) {

    if(_mLastResult.error()) {
        return *this;
    }

    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    if(_mLastResult.error()) {
        return *this;
    }

    if((_mCurrentChar < '0' || _mCurrentChar > '9') && _mCurrentChar != '-') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        return *this;
    }

    nbl::JsonValue value;
    _ParseContext context {
        .reader = _mReader,
        .storage = nullptr,
        .temporaryStorage = nullptr,
        .outValue = &value,
        .lastReadChar = _mCurrentChar
    };
    _mLastResult = _parseNumber(&context);
    if(_mLastResult.error()) {
        return *this;
    }
    _mCurrentChar = context.lastReadChar;

    if(context.outValue->type != nbl::JsonType::NUMBER) {
        _mLastResult = nbl::error::UNEXPECTED_ERROR;
        return *this;
    }

    *outNumber = context.outValue->number;
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::readBoolean(bool* outBoolean) {

    if(_mLastResult.error()) {
        return *this;
    }

    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    if(_mLastResult.error()) {
        return *this;
    }

    if(_mCurrentChar == 't') {
        char buffer[3];
        _mLastResult = _mReader.readAll(buffer, 3);
        if(_mLastResult.error()) {
            return *this;
        }
        if(buffer[0] == 'r' && buffer[1] == 'u' && buffer[2] == 'e') {
            *outBoolean = true;
            _mLastResult = _nextToken(_mReader, &_mCurrentChar);
            return *this;
        }
    } else if(_mCurrentChar == 'f') {
        char buffer[4];
        _mLastResult = _mReader.readAll(buffer, 4);
        if(_mLastResult.error()) {
            return *this;
        }
        if(buffer[0] == 'a' && buffer[1] == 'l' && buffer[2] == 's' && buffer[3] == 'e') {
            *outBoolean = false;
            _mLastResult = _nextToken(_mReader, &_mCurrentChar);
            return *this;
        }
    }

    _mLastResult = nbl::error::UNEXPECTED_TOKEN;
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::beginArray() {
    
    if(_mLastResult.error()) {
        return *this;
    }
    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    if(_mLastResult.error()) {
        return *this;
    } else if(_mCurrentChar != '[') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        return *this;
    }

    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::endArray() {

    if(_mLastResult.error()) {
        return *this;
    }
    if(_mCurrentChar != ']') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
    }
    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::readNull() {

    if(_mLastResult.error()) {
        return *this;
    }

    _mLastResult = _nextToken(_mReader, &_mCurrentChar);
    if(_mLastResult.error()) {
        return *this;
    }

    if(_mCurrentChar != 'n') {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
        return *this;
    }

    char buffer[3];
    _mLastResult = _mReader.readAll(buffer, 3);
    if(_mLastResult.error()) {
        return *this;
    }
    if(buffer[0] == 'u' && buffer[1] == 'l' && buffer[2] == 'l') {
        _mLastResult = _nextToken(_mReader, &_mCurrentChar);
        return *this;
    }
    
    _mLastResult = nbl::error::UNEXPECTED_TOKEN;
    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::skipKey() {
    if(_mLastResult.error()) {
        return *this;
    }

    if(_mCurrentChar == '{' || _mCurrentChar == ',') {
        _mLastResult = _nextToken(_mReader, &_mCurrentChar);
        if(_mLastResult.error()) {
            return *this;
        } else if(_mCurrentChar != '"') {
            _mLastResult = nbl::error::UNEXPECTED_TOKEN;
            return *this;
        } else {
            _mLastResult = _readString(_mReader, nullptr, &_mCurrentChar);
            if(_mLastResult.error()) {
                return *this;
            } else if(_mCurrentChar != ':') {
                _mLastResult = nbl::error::UNEXPECTED_TOKEN;
                return *this;
            }
        }
    } else {
        _mLastResult = nbl::error::UNEXPECTED_TOKEN;
    }

    return *this;
}

nbl::PartialJsonParser& nbl::PartialJsonParser::skipValue() {

    if(_mLastResult.error()) {
        return *this;
    }

    nbl::JsonValue dummy;
    _ParseContext context {
        .reader = _mReader,
        .storage = nullptr,
        .temporaryStorage = nullptr,
        .outValue = &dummy,
        .lastReadChar = _mCurrentChar
    };

    _mLastResult = _nextToken(&context);
    if(_mLastResult.error()) {
        return *this;
    }

    _mLastResult = _parseValue(&context);
    _mCurrentChar = context.lastReadChar;

    return *this;
}
