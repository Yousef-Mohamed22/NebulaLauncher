/*
 * Copyright 2026 jwall64
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
 * \file JsonParser.cpp
 * \author jwall64
 * \brief Partial implementation of Codec/Json.hpp
 */

#include <Codec/Json.hpp>

#include <Core/Span.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>
#include <Codec/Utf8.hpp>
#include <Platform/Allocator.hpp>

namespace {

struct _ParseContext {
    nbl::Reader reader;
    nbl::LinearAllocator* storage;
    nbl::LinearAllocator* tempStorage;
    nbl::JsonElement outElement;
    usize bufferIndex;
    usize bufferLength;
    usize bufferCapacity;
    char* buffer;

    char lastRead() { 
        return buffer[bufferIndex];
    }

    nbl::Result nextChar() {
        if(bufferIndex + 1 >= bufferLength) [[unlikely]] {
            if(reader.pfnRead == nullptr) return nbl::Result::UNEXPECTED_END_OF_STREAM;
            usize bytesRead = 0;
            while(bytesRead == 0) {
                nbl::Result res = reader.read(buffer, bufferCapacity, &bytesRead);
                if(res.error()) return res;
                if(res == nbl::Result::END_OF_STREAM && bytesRead == 0) {
                    return nbl::Result::UNEXPECTED_END_OF_STREAM;
                }
            }
            bufferLength = bytesRead;
            bufferIndex = 0;
        } else {
            bufferIndex++;
        }
        return nbl::Result::SUCCESS;
    }

    nbl::Result nextChars(usize count, const char** read) {
        NBL_ASSERT(count <= bufferCapacity);
        if(bufferIndex + count >= bufferLength) [[unlikely]] {
            if(reader.pfnRead == nullptr) return nbl::Result::UNEXPECTED_END_OF_STREAM;

            usize toSave = bufferLength - bufferIndex;
            if(toSave > 0) {
                memmove(buffer, buffer + bufferIndex, toSave);
            }

            bufferLength = 0;

            usize bytesRead = 0;
            while(bufferLength < count) {
                nbl::Result res = reader.read(buffer + bufferLength, 4096 - bufferLength, &bytesRead);
                if(res.error()) return res;
                bufferLength += bytesRead;

                if(res == nbl::Result::END_OF_STREAM && bufferLength < count) {
                    return nbl::Result::UNEXPECTED_END_OF_STREAM;
                }
            }

            *read = buffer;
            bufferIndex = count - 1;
        } else {
            *read = buffer + bufferIndex + 1;
            bufferIndex += count;
        }
            

        return nbl::Result::SUCCESS;
    }

};

i32 _hexDigit(char c) {
    if(c >= '0' && c <= '9') return c - '0';
    if(c >= 'a' && c <= 'f') return c - 'a' + 10;
    if(c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

bool _isWhitespace(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

nbl::Result _nextToken(_ParseContext* context) {
    do {
        nbl::Result res = context->nextChar();
        if(res.error()) return res;
    } while(_isWhitespace(context->lastRead()));
    return nbl::Result::SUCCESS;
}

nbl::Result _readString(_ParseContext* context, nbl::LinearAllocator* storage, nbl::StringView* out) {

    if(context->lastRead() != '"') {
        NBL_DEBUG("Found invalid character ", context->lastRead(), "while attempting to parse a JSON string");
        return nbl::Result::UNEXPECTED_TOKEN;
    }
    char* begin = static_cast<char*>(storage->end());

    while(true) {
        if(context->lastRead() < 0x20) {
            return nbl::Result::PARSE_ERROR;
        }

        NBL_CHECK(context->nextChar());
        char c = context->lastRead();

        if(c == '\\') {
            NBL_CHECK(context->nextChar());
            char next = context->lastRead();

            switch(next) {
                case '"': {
                    NBL_CHECK(storage->write("\"", 1));
                    break;
                }
                case '\\': {
                    NBL_CHECK(storage->write("\\", 1));
                    break;
                }
                case '/': {
                    NBL_CHECK(storage->write("/", 1));
                    break;
                }
                case 'b': {
                    NBL_CHECK(storage->write("\b", 1));
                    break;
                }
                case 'f': {
                    NBL_CHECK(storage->write("\f", 1));
                    break;
                }
                case 'n': {
                    NBL_CHECK(storage->write("\n", 1));
                    break;
                }
                case 'r': {
                    NBL_CHECK(storage->write("\r", 1));
                    break;
                }
                case 't': {
                    NBL_CHECK(storage->write("\t", 1));
                    break;
                }
                case 'u': {
                    const char* codePointHex;
                    NBL_CHECK(context->nextChars(4, &codePointHex));

                    u32 codePoint = 0;
                    for(auto i = 0 ; i < 4 ; i++) {
                        i32 val = _hexDigit(codePointHex[i]);
                        if(val == -1) return nbl::Result::PARSE_ERROR;
                        codePoint |= (val & 0xF) << ((3 - i) * 4);
                    }

                    // Surrogate pairs: a high surrogate (0xD800-0xDBFF) must be followed by a
                    // low surrogate (0xDC00-0xDFFF); the pair encodes a single code point > 0xFFFF.
                    if(codePoint >= 0xD800 && codePoint <= 0xDBFF) {

                        const char* lowHalf;
                        NBL_CHECK(context->nextChars(6, &lowHalf));

                        if(lowHalf[0] != '\\') return nbl::Result::PARSE_ERROR;
                        if(lowHalf[1] != 'u') return nbl::Result::PARSE_ERROR;

                        const char* lowHex = lowHalf + 2;
                        i32 lowSurrogate = 0;
                        for(auto i = 0 ; i < 4 ; i++) {
                            i32 val = _hexDigit(lowHex[i]);
                            if(val == -1) return nbl::Result::PARSE_ERROR;
                            lowSurrogate |= (val & 0xF) << ((3 - i) * 4);
                        }

                        if(lowSurrogate < 0xDC00 || lowSurrogate > 0xDFFF) {
                            return nbl::Result::PARSE_ERROR;
                        }

                        codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
                    }

                    char unicode[4]{};
                    nbl::BufferStream buf(unicode, 4);
                    NBL_CHECK(nbl::writeCodePoint(buf, codePoint));

                    storage->write(unicode, buf.writerIndex());
                    break;
                }
                default: {
                    // Only the JSON-specified escape sequences are valid; anything else is an error.
                    return nbl::Result::PARSE_ERROR;
                }
            }
        }
        else if(c == '\"') {
            break;
        } 
        else {
            storage->write(&c, 1);
        }
    }

    char nt = 0;
    storage->write(&nt, 1);

    char* last = static_cast<char*>(storage->end());
    usize length = last - begin;

    usize alignedLength = nbl::alignUp(length, sizeof(void*));
    usize padding = alignedLength - length;

    if(padding > 0) {
        void* paddingData = nullptr;
        NBL_CHECK(storage->push(padding, &paddingData));
        memset(paddingData, 0, padding);
    }

    out->length = length - 1;
    out->data = begin;
    out->nullTerminated = true;

    return nbl::Result::SUCCESS;
}

nbl::Result _parseString(_ParseContext* context) {
    NBL_CHECK(_readString(context, context->storage, &context->outElement.string));

    nbl::Result res = _nextToken(context);
    if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

    context->outElement.type = nbl::JsonType::STRING;

    return nbl::Result::SUCCESS;
}

nbl::Result _parseElement(_ParseContext* context);

nbl::Result _parseArrayInPlace(_ParseContext* context, usize depth = 0) {

    nbl::JsonElement elements[128] = {};
    usize startIndex = 0;
    usize elementCount = 0;
    bool done = false;
    while(true) {

        NBL_CHECK(_parseElement(context));

        elements[elementCount++] = context->outElement;

        if(context->lastRead() == ']') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

            done = true;
            break;
        }
        if(context->lastRead() != ',') {
            NBL_DEBUG("Found invalid character ", context->lastRead(), " after parsing an array element (in place)");
            return nbl::Result::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(_nextToken(context));
        if(elementCount >= 128) break;
    }

    if(done) {

        usize totalElements = elementCount + (depth * 128);
        context->outElement.type = nbl::JsonType::ARRAY;

        void* outElements;
        NBL_CHECK(context->storage->push(totalElements * sizeof(nbl::JsonElement), &outElements));

        context->outElement.array.elements = static_cast<nbl::JsonElement*>(outElements);
        context->outElement.array.count = totalElements;
        
    } else {
        NBL_CHECK(_parseArrayInPlace(context, depth + 1));
    }

    memcpy(context->outElement.array.elements + (depth * 128), elements, elementCount * sizeof(nbl::JsonElement));
    return nbl::Result::SUCCESS;
}

nbl::Result _parseArray(_ParseContext* context) {

    if(context->lastRead() != '[') {
        NBL_DEBUG("Found invalid character ", context->lastRead(), " while trying to parse an array");
        return nbl::Result::UNEXPECTED_TOKEN;
    }

    NBL_CHECK(_nextToken(context));
    if(context->lastRead() == ']') {

        nbl::Result res = _nextToken(context);
        if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

        context->outElement = nbl::JsonArray {
            .elements = nullptr,
            .count = 0
        };

        return nbl::Result::SUCCESS;
    }

    if(context->tempStorage == nullptr) return _parseArrayInPlace(context);

    nbl::AllocationFrame frame(nbl::Arena::threadLocal());

    void* firstElement = context->tempStorage->end();
    usize elements = 0;
    while(true) {

        NBL_CHECK(_parseElement(context));
        NBL_CHECK(frame.write(&context->outElement, sizeof(nbl::JsonElement)));

        elements++;

        if(context->lastRead() == ']') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

            void* outElements = nullptr;
            res = context->storage->push(elements * sizeof(nbl::JsonElement), &outElements);
            if(res.error()) return res;

            memcpy(outElements, firstElement, elements * sizeof(nbl::JsonElement));

            context->outElement = nbl::JsonArray {
                .elements = static_cast<nbl::JsonElement*>(outElements),
                .count = elements
            };

            return nbl::Result::SUCCESS;
        }
        if(context->lastRead() != ',') {
            NBL_DEBUG("Found invalid character ", context->lastRead(), " after parsing an array element");
            return nbl::Result::UNEXPECTED_TOKEN;
        }
        NBL_CHECK(_nextToken(context));
    }
}


void _fillBuckets(nbl::ConstSpan<nbl::JsonObject::Entry> unorderedEntries, nbl::JsonObject* obj) {

    NBL_ASSERT(unorderedEntries.length == obj->entryCount);

    const nbl::JsonObject::Entry* source = unorderedEntries.data;
    if(source == obj->entries) {
        source = obj->entries + obj->entryCount;
        memcpy(obj->entries + obj->entryCount, obj->entries, sizeof(nbl::JsonObject::Entry) * obj->entryCount);
    }

    for(usize i = 0 ; i < obj->entryCount ; i++) {
        usize bucket = source[i].key.hash() % obj->bucketCount;
        obj->buckets[bucket].count++;
    }

    usize currentOffset = 0;
    for(usize i = 0 ; i < obj->bucketCount ; i++) {
        obj->buckets[i].offset = currentOffset;
        currentOffset += obj->buckets[i].count;
    }

    for(usize i = 0 ; i < obj->entryCount ; i++) {
        usize bucket = source[i].key.hash() % obj->bucketCount;
        obj->entries[obj->buckets[bucket].offset++] = source[i];
    }

    for(usize i = 0 ; i < obj->bucketCount ; i++) {
        obj->buckets[i].offset -= obj->buckets[i].count;
    }
}


nbl::Result _parseObjectInPlace(_ParseContext* context, usize depth = 0) {

    nbl::JsonObject::Entry entries[128] = {};
    usize entryCount = 0;
    bool done = false;
    while(true) {

        nbl::JsonObject::Entry nextEntry = {};
        
        NBL_CHECK(_parseString(context));
        if(context->lastRead() != ':') {
            NBL_DEBUG("Found invalid character ", context->lastRead(), " after parsing an object key (in place)");
            return nbl::Result::UNEXPECTED_TOKEN;
        }

        nextEntry.key = context->outElement.string;

        NBL_CHECK(_nextToken(context));
        NBL_CHECK(_parseElement(context));

        nextEntry.value = context->outElement;
        entries[entryCount++] = nextEntry;

        if(context->lastRead() == '}') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

            done = true;
            break;
        }
        if(context->lastRead() != ',') {
            NBL_DEBUG("Found invalid character ", context->lastRead(), " after parsing an object value (in place) bufferIndex: ", context->bufferIndex);
            return nbl::Result::UNEXPECTED_TOKEN;
        }

        NBL_CHECK(_nextToken(context));
        if(entryCount >= 128) break;
    }


    if(done) {

        usize totalEntries = entryCount + (depth * 128);
        usize bucketCount = totalEntries == 1 ? 1 : totalEntries * 2;
        usize startIndex = context->storage->index();

        context->outElement.type = nbl::JsonType::OBJECT;

        void* outBuckets;
        NBL_CHECK(context->storage->push(bucketCount * sizeof(nbl::JsonObject::Bucket), &outBuckets));

        void* outElements;
        nbl::Result res = context->storage->push(totalEntries * 2 * sizeof(nbl::JsonObject::Entry), &outElements);
        if(res.error()) {
            NBL_CHECK(context->storage->popTo(startIndex));
            return res;
        }


        context->outElement.object.entries = static_cast<nbl::JsonObject::Entry*>(outElements);
        context->outElement.object.entryCount = totalEntries;

        context->outElement.object.buckets = static_cast<nbl::JsonObject::Bucket*>(outBuckets);
        context->outElement.object.bucketCount = bucketCount;

    } else {
        NBL_CHECK(_parseObjectInPlace(context, depth + 1));
    }

    memcpy(context->outElement.object.entries + (depth * 128), entries, entryCount * sizeof(nbl::JsonObject::Entry));

    if(depth == 0) {
        _fillBuckets(nbl::ConstSpan(context->outElement.object.entries, context->outElement.object.entryCount), &context->outElement.object);
        NBL_CHECK(context->storage->pop(context->outElement.object.entryCount * sizeof(nbl::JsonObject::Entry)));
    }

    return nbl::Result::SUCCESS;
}

nbl::Result _parseObject(_ParseContext* context) {

    if(context->lastRead() != '{') return nbl::Result::UNEXPECTED_TOKEN;

    NBL_CHECK(_nextToken(context));
    if(context->lastRead() == '}') {

        nbl::Result res = _nextToken(context);
        if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

        context->outElement = nbl::JsonObject {
            .entries = nullptr,
            .entryCount = 0,
            .buckets = nullptr,
            .bucketCount = 0
        };
        return nbl::Result::SUCCESS;
    }

    if(context->tempStorage == nullptr) return _parseObjectInPlace(context);

    nbl::JsonObject::Entry* firstEntry = static_cast<nbl::JsonObject::Entry*>(context->tempStorage->end());
    usize entries = 0;

    nbl::JsonObject::Entry* outElements = nullptr;
    nbl::JsonObject::Bucket* outBuckets = nullptr;

    while(true) {

        nbl::JsonObject::Entry nextEntry = {};
        
        NBL_CHECK(_parseString(context));

        if(context->lastRead() != ':') {
            NBL_DEBUG("Found invalid character ", context->lastRead(), " after parsing an object key");
            return nbl::Result::UNEXPECTED_TOKEN;
        }

        nextEntry.key = context->outElement.string;

        NBL_CHECK(_nextToken(context));
        NBL_CHECK(_parseElement(context));

        nextEntry.value = context->outElement;

        NBL_CHECK(context->tempStorage->write(&nextEntry, sizeof(nbl::JsonObject::Entry)));

        entries++;

        if(context->lastRead() == '}') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

            // bucketCount is only known now that all entries have been counted.
            usize bucketCount = entries * 2;

            NBL_CHECK(context->storage->push(entries * sizeof(nbl::JsonObject::Entry), reinterpret_cast<void**>(&outElements)));
            NBL_CHECK(context->storage->push(bucketCount * sizeof(nbl::JsonObject::Bucket), reinterpret_cast<void**>(&outBuckets)));

            context->outElement = nbl::JsonObject {
                .entries = outElements,
                .entryCount = entries,
                .buckets = outBuckets,
                .bucketCount = bucketCount
            };
            _fillBuckets(nbl::ConstSpan<nbl::JsonObject::Entry>(firstEntry, entries), &context->outElement.object);

            return nbl::Result::SUCCESS;
        }
        if(context->lastRead() != ',') {
            NBL_DEBUG("Found invalid character ", context->lastRead(), " after parsing an object value");
            return nbl::Result::UNEXPECTED_TOKEN;
        }
        NBL_CHECK(_nextToken(context));
    }
}


nbl::Result _parseExponentPart(_ParseContext* context, u64 wholePart, f64 decimalPart, bool negative) {

    bool expNegative = false;
    NBL_CHECK(context->nextChar());
    if(context->lastRead() == '-') {
        expNegative = true;
        NBL_CHECK(context->nextChar());
    } else if(context->lastRead() == '+') {
        NBL_CHECK(context->nextChar());
    }

    if(context->lastRead() < '0' || context->lastRead() > '9') {
        return nbl::Result::PARSE_ERROR;
    }

    u64 exponent = 0;
    while(true) {
        if(context->lastRead() >= '0' && context->lastRead() <= '9') {
            exponent *= 10;
            exponent += context->lastRead() - '0';

            nbl::Result res = context->nextChar();
            if(res.error()) {
                if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
                break;
            }
            continue;
        }
        else if(_isWhitespace(context->lastRead())) {
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            break;
        }
        else {
            break;
        }
    }


    if(decimalPart != 0.0) {
        f64 finalValue = static_cast<f64>(wholePart) + decimalPart;
        if(negative) finalValue = -finalValue;

        if(expNegative) {
            for(usize i = 0 ; i < exponent ; i++) {
                finalValue /= 10.0;
            }
        } else {
            for(usize i = 0 ; i < exponent ; i++) {
                finalValue *= 10.0;
            }
        }

        context->outElement = nbl::JsonNumber {
            .type = nbl::JsonNumberType::FLOAT,
            .floatValue = finalValue
        };
    } else {
        if(expNegative) {
            f64 finalValue = static_cast<f64>(wholePart);
            for(usize i = 0 ; i < exponent ; i++) {
                finalValue /= 10.0;
            }
            if(negative) finalValue = -finalValue;
            context->outElement = nbl::JsonNumber {
                .type = nbl::JsonNumberType::FLOAT,
                .floatValue = finalValue
            };
        } else {
            for(usize i = 0 ; i < exponent ; i++) {
                wholePart *= 10;
            }

            if(negative) {
                context->outElement = nbl::JsonNumber {
                    .type = nbl::JsonNumberType::SIGNED,
                    .intValue = -static_cast<i64>(wholePart)
                };
            } else {
                context->outElement = nbl::JsonNumber {
                    .type = nbl::JsonNumberType::UNSIGNED,
                    .uintValue = wholePart
                };
            }
        }
    }

    return nbl::Result::SUCCESS;
}

nbl::Result _parseDecimalPart(_ParseContext* context, u64 wholePart, bool negative) {

    NBL_CHECK(context->nextChar());
    if(context->lastRead() < '0' || context->lastRead() > '9') {
        return nbl::Result::PARSE_ERROR;
    }

    usize digits = 1;
    u64 decimalPart = context->lastRead() - '0';
    
    while(true) {
        nbl::Result res = context->nextChar();
        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead() == 'e' || context->lastRead() == 'E') {
                return _parseExponentPart(context, wholePart, static_cast<f64>(decimalPart) / (digits * 10), negative);
            }
            else if(context->lastRead() >= '0' && context->lastRead() <= '9') {
                decimalPart *= 10;
                decimalPart += context->lastRead() - '0';
                digits++;
                continue;
            }
            else if(_isWhitespace(context->lastRead())) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }

        f64 after = static_cast<f64>(decimalPart);
        for(auto i = 0 ; i < digits ; i++) {
            after /= 10.0;
        }
        f64 finalValue = static_cast<f64>(wholePart) + after;
        if(negative) finalValue = -finalValue;
        context->outElement = nbl::JsonNumber {
            .type = nbl::JsonNumberType::FLOAT,
            .floatValue = finalValue
        };

        return nbl::Result::SUCCESS;
    }
}


nbl::Result _parseNumber(_ParseContext* context) {

    bool negative = false;
    if(context->lastRead() == '-') {
        negative = true;
        NBL_CHECK(context->nextChar());
    }

    if(context->lastRead() == '0') {
        // Zero or decimal

        nbl::Result res = context->nextChar();
        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead() >= '1' && context->lastRead() <= '9') {
                return nbl::Result::PARSE_ERROR;
            }
            if(context->lastRead() == '.') {
                return _parseDecimalPart(context, 0, negative);
            }
            else if(context->lastRead() == 'e' || context->lastRead() == 'E') {
                return _parseExponentPart(context, 0, 0.0, negative);
            }
            else if(_isWhitespace(context->lastRead())) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }
        context->outElement = nbl::JsonNumber {
            .type = nbl::JsonNumberType::SIGNED,
            .intValue = 0
        };
        return nbl::Result::SUCCESS;
    }

    if(context->lastRead() < '1' || context->lastRead() > '9') {
        return nbl::Result::PARSE_ERROR;
    }

    u64 wholePart = context->lastRead() - '0';

    while(true) {
        nbl::Result res = context->nextChar();

        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead() == '.') {
                return _parseDecimalPart(context, wholePart, negative);
            }
            else if(context->lastRead() == 'e' || context->lastRead() == 'E') {
                return _parseExponentPart(context, wholePart, 0.0, negative);
            }
            else if(context->lastRead() >= '0' && context->lastRead() <= '9') {
                wholePart *= 10;
                wholePart += context->lastRead() - '0';
                continue;
            }
            else if(_isWhitespace(context->lastRead())) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }


        if(negative) {
            context->outElement = nbl::JsonNumber {
                .type = nbl::JsonNumberType::SIGNED, 
                .intValue = -static_cast<i64>(wholePart)
            };
        } else {
            context->outElement = nbl::JsonNumber {
                .type = nbl::JsonNumberType::UNSIGNED, 
                .uintValue = wholePart
            };
        }

        return nbl::Result::SUCCESS;
    }
}

nbl::Result _parseElement(_ParseContext* context) {

    char c = context->lastRead();
    switch(c) {
        case 'n': { // null
            const char* remaining = nullptr;
            if(context->bufferIndex + 3 >= context->bufferLength) {
                return nbl::Result::UNEXPECTED_END_OF_STREAM;
            }
            NBL_CHECK(context->nextChars(3, &remaining));
            if(memcmp(remaining, "ull", 3) != 0) {
                return nbl::Result::PARSE_ERROR;
            }
            context->outElement.type = nbl::JsonType::NONE;
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            return nbl::Result::SUCCESS;
        }
        case 't': { // true
            const char* remaining = nullptr;
            if(context->bufferIndex + 3 >= context->bufferLength) {
                return nbl::Result::UNEXPECTED_END_OF_STREAM;
            }
            NBL_CHECK(context->nextChars(3, &remaining));
            if(memcmp(remaining, "rue", 3) != 0) {
                return nbl::Result::PARSE_ERROR;
            }
            context->outElement.type = nbl::JsonType::BOOLEAN;
            context->outElement.boolean = true;
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            return nbl::Result::SUCCESS;
        }
        case 'f': { // false
            const char* remaining = nullptr;
            if(context->bufferIndex + 4 >= context->bufferLength) {
                return nbl::Result::UNEXPECTED_END_OF_STREAM;
            }
            NBL_CHECK(context->nextChars(4, &remaining));
            if(memcmp(remaining, "alse", 4) != 0) {
                return nbl::Result::PARSE_ERROR;
            }
            context->outElement.type = nbl::JsonType::BOOLEAN;
            context->outElement.boolean = false;
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            return nbl::Result::SUCCESS;
        }
        case '"': return _parseString(context);
        case '[': return _parseArray(context); 
        case '{': return _parseObject(context);
        default:
            if(c >= '0' && c <= '9' || c == '-') return _parseNumber(context);
            NBL_DEBUG("Found invalid character ", c, "while attempting to parse a JSON element");
            return nbl::Result::UNEXPECTED_TOKEN;
    }
}

}


nbl::Result nbl::parseJson(nbl::Reader reader, nbl::LinearAllocator* storage, nbl::JsonElement* outElement, nbl::LinearAllocator* tempStorage) {

    if(tempStorage == storage || storage == nullptr) return nbl::Result::INVALID_ARGUMENT;

    char buffer[4096]{};

    _ParseContext ctx = {
        .reader = reader,
        .storage = storage,
        .tempStorage = tempStorage,
        .outElement = {},
        .bufferIndex = 0,
        .bufferLength = 0,
        .bufferCapacity = 4096,
        .buffer = buffer
    };


    nbl::Result res = reader.read(buffer, 4096, &ctx.bufferLength);
    if(res.error()) return res;
    if(res == nbl::Result::END_OF_STREAM && ctx.bufferLength == 0) {
        return nbl::Result::UNEXPECTED_END_OF_STREAM;
    }

    while(_isWhitespace(ctx.lastRead())) {
        NBL_CHECK(ctx.nextChar());
    }
    NBL_CHECK(_parseElement(&ctx));

    if(outElement) *outElement = ctx.outElement;
    return nbl::Result::SUCCESS;

}

nbl::Result nbl::parseJson(nbl::StringView str, nbl::LinearAllocator* storage, nbl::JsonElement* outElement, nbl::LinearAllocator* tempStorage) {

    if(tempStorage == storage || storage == nullptr) return nbl::Result::INVALID_ARGUMENT;
    if(str.length == 0) return Result::UNEXPECTED_END_OF_STREAM;

    _ParseContext ctx = {
        .reader = {},
        .storage = storage,
        .tempStorage = tempStorage,
        .outElement = {},
        .bufferIndex = 0,
        .bufferLength = str.length,
        .bufferCapacity = str.length,
        .buffer = const_cast<char*>(str.data)
    };

    while(_isWhitespace(ctx.lastRead())) {
        NBL_CHECK(ctx.nextChar());
    }
    NBL_CHECK(_parseElement(&ctx));

    if(outElement) *outElement = ctx.outElement;
    return nbl::Result::SUCCESS;

}

nbl::Result nbl::parseJsonNumber(Reader reader, JsonNumber* outNumber) {

    char buffer[256]{};

    _ParseContext ctx = {
        .reader = reader,
        .storage = nullptr,
        .tempStorage = nullptr,
        .outElement = {},
        .bufferIndex = 0,
        .bufferLength = 0,
        .bufferCapacity = 256,
        .buffer = buffer
    };

    nbl::Result res = reader.read(buffer, 256, &ctx.bufferLength);
    if(res.error()) return res;
    if(res == nbl::Result::END_OF_STREAM && ctx.bufferLength == 0) {
        return nbl::Result::UNEXPECTED_END_OF_STREAM;
    }

    if(ctx.lastRead() >= '0' && ctx.lastRead() <= '9' || ctx.lastRead() == '-') {
        NBL_CHECK(_parseNumber(&ctx));

        if(outNumber) *outNumber = ctx.outElement.number;
        return nbl::Result::SUCCESS;
    }

    return nbl::Result::UNEXPECTED_TOKEN;
}
