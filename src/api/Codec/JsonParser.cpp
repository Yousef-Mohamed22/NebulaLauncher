#include <Core/Span.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Core/Math.hpp>
#include <Platform/Logging.hpp>
#include <Codec/Utf8.hpp>
#include <Codec/Json.hpp>
#include "Platform/Allocator.hpp"

namespace {

struct _ParseContext {
    nbl::Reader reader;
    nbl::LinearAllocator* storage;
    nbl::LinearAllocator* tempStorage;
    nbl::JsonElement outElement;
    char lastRead;
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
        nbl::Result res = context->reader.readAll(&context->lastRead, 1);
        if(res.error()) return res;
    } while(_isWhitespace(context->lastRead));
    return nbl::Result::SUCCESS;
}

nbl::Result _readString(nbl::Reader reader, char* lastRead, nbl::LinearAllocator* storage, nbl::StringView* out) {

    if(*lastRead != '"') return nbl::Result::UNEXPECTED_TOKEN;
    char* begin = static_cast<char*>(storage->end());

    char buffer[1024];
    usize bufferLength = 0;

    auto _append = [&](char c) -> nbl::Result {
        buffer[bufferLength++] = c;
        if(bufferLength == 1024) {
            NBL_CHECK(storage->write(buffer, 1024));
            bufferLength = 0;
        }
        return nbl::Result::SUCCESS;
    };

    while(true) {
        NBL_CHECK(reader.readAll(lastRead, 1));
        if(*lastRead >= 0 && *lastRead < 0x20) {
            nbl::log(nbl::LogLevel::DEBUG, "Found control character: ", static_cast<i32>(*lastRead));
            return nbl::Result::PARSE_ERROR;
        }

        char c = *lastRead;
        if(c == '\\') {
            NBL_CHECK(reader.readAll(lastRead, 1));
            char next = *lastRead;

            switch(next) {
                case '"': {
                    NBL_CHECK(_append('"'));
                    break;
                }
                case '\\': {
                    NBL_CHECK(_append('\\'));
                    break;
                }
                case '/': {
                    NBL_CHECK(_append('/'));
                    break;
                }
                case 'b': {
                    NBL_CHECK(_append('\b'));
                    break;
                }
                case 'f': {
                    NBL_CHECK(_append('\f'));
                    break;
                }
                case 'n': {
                    NBL_CHECK(_append('\n'));
                    break;
                }
                case 'r': {
                    NBL_CHECK(_append('\r'));
                    break;
                }
                case 't': {
                    NBL_CHECK(_append('\t'));
                    break;
                }
                case 'u': {
                    char codePointHex[4]{};
                    NBL_CHECK(reader.readAll(codePointHex, 4));

                    i32 codePoint = 0;
                    for(i32 i = 0 ; i < 4 ; i++) {
                        i32 val = _hexDigit(codePointHex[i]);
                        if(val == -1) {
                            nbl::log(nbl::LogLevel::DEBUG, "Invalid hex digit: ", static_cast<i32>(codePointHex[i]));
                            return nbl::Result::PARSE_ERROR;
                        }
                        codePoint |= (val & 0xF) << ((3 - i) * 4);
                    }

                    nbl::BufferStream buf(codePointHex, 4);
                    nbl::writeCodePoint(buf, codePoint);

                    if(bufferLength > 1024 - buf.writerIndex() - 1) {
                        NBL_CHECK(storage->write(buffer, 1024));
                        bufferLength = 0;
                    }

                    memcpy(buffer + bufferLength, codePointHex, buf.writerIndex());
                    bufferLength += buf.writerIndex();
                    break;
                }
            }
        }
        else if(c == '\"') {
            break;
        } 
        else {
            buffer[bufferLength++] = c;
            if(bufferLength == 1024) {
                NBL_CHECK(storage->write(buffer, 1024));
                bufferLength = 0;
            }
        }
    }

    if(bufferLength > 0) {
        NBL_CHECK(storage->write(buffer, bufferLength));
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
    NBL_CHECK(_readString(context->reader, &context->lastRead, context->storage, &context->outElement.string));

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

        if(context->lastRead == ']') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

            done = true;
            break;
        }
        if(context->lastRead != ',') {
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

    if(context->lastRead != '[') return nbl::Result::UNEXPECTED_TOKEN;

    NBL_CHECK(_nextToken(context));
    if(context->lastRead == ']') {
        context->outElement = nbl::JsonArray{
            .elements = nullptr,
            .count = 0
        };
        nbl::Result res = _nextToken(context);
        if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        return nbl::Result::SUCCESS;
    }

    if(context->tempStorage == nullptr) return _parseArrayInPlace(context);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    void* firstElement = context->tempStorage->end();
    usize elements = 0;
    while(true) {

        NBL_CHECK(_parseElement(context));
        NBL_CHECK(frame.write(&context->outElement, sizeof(nbl::JsonElement)));

        elements++;

        if(context->lastRead == ']') {

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
        if(context->lastRead != ',') {
            return nbl::Result::UNEXPECTED_TOKEN;
        }
        NBL_CHECK(_nextToken(context));
    }
}


void _fillBuckets(nbl::ConstSpan<nbl::JsonObject::Entry> unorderedEntries, nbl::JsonObject* obj) {

    NBL_ASSERT(unorderedEntries.length == obj->entryCount);

    for(usize i = 0 ; i < obj->entryCount ; i++) {
        usize bucket = unorderedEntries[i].key.hash() % obj->bucketCount;
        obj->buckets[bucket].count++;
    }

    usize currentOffset = 0;
    for(usize i = 0 ; i < obj->bucketCount ; i++) {
        obj->buckets[i].offset = currentOffset;
        currentOffset += obj->buckets[i].count;
    }

    for(usize i = 0 ; i < obj->entryCount ; i++) {
        usize bucket = unorderedEntries[i].key.hash() % obj->bucketCount;
        obj->entries[obj->buckets[bucket].offset++] = unorderedEntries[i];
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
        if(context->lastRead != ':') {
            return nbl::Result::UNEXPECTED_TOKEN;
        }

        nextEntry.key = context->outElement.string;

        NBL_CHECK(_nextToken(context));
        NBL_CHECK(_parseElement(context));

        nextEntry.value = context->outElement;
        entries[entryCount++] = nextEntry;

        if(context->lastRead == '}') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

            done = true;
            break;
        }
        if(context->lastRead != ',') {
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

    memcpy(context->outElement.object.entries + context->outElement.object.entryCount + (depth * 128), entries, entryCount * sizeof(nbl::JsonObject::Entry));

    if(depth == 0) {
        _fillBuckets(nbl::ConstSpan(context->outElement.object.entries + context->outElement.object.entryCount, context->outElement.object.entryCount), &context->outElement.object);
        NBL_CHECK(context->storage->pop(context->outElement.object.entryCount * sizeof(nbl::JsonObject::Entry)));
    }

    return nbl::Result::SUCCESS;
}

nbl::Result _parseObject(_ParseContext* context) {

    if(context->lastRead != '{') return nbl::Result::UNEXPECTED_TOKEN;

    NBL_CHECK(_nextToken(context));
    if(context->lastRead == '}') {
        context->outElement = nbl::JsonObject {
            .entries = nullptr,
            .entryCount = 0,
            .buckets = nullptr,
            .bucketCount = 0
        };
        nbl::Result res = _nextToken(context);
        if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        return nbl::Result::SUCCESS;
    }

    if(context->tempStorage == nullptr) return _parseObjectInPlace(context);

    nbl::JsonObject::Entry* firstEntry = static_cast<nbl::JsonObject::Entry*>(context->tempStorage->end());
    usize entries = 0;

    usize bucketCount = entries * 2;
    nbl::JsonObject::Entry* outElements = nullptr;
    nbl::JsonObject::Bucket* outBuckets = nullptr;

    while(true) {

        nbl::JsonObject::Entry nextEntry = {};
        
        NBL_CHECK(_parseString(context));

        if(context->lastRead != ':') {
            return nbl::Result::UNEXPECTED_TOKEN;
        }

        nextEntry.key = context->outElement.string;

        NBL_CHECK(_nextToken(context));
        NBL_CHECK(_parseElement(context));

        nextEntry.value = context->outElement;

        NBL_CHECK(context->tempStorage->write(&nextEntry, sizeof(nbl::JsonObject::Entry)));

        entries++;

        if(context->lastRead == '}') {

            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;

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
        if(context->lastRead != ',') {
            return nbl::Result::UNEXPECTED_TOKEN;
        }
        NBL_CHECK(_nextToken(context));
    }
}


nbl::Result _parseExponentPart(_ParseContext* context, u64 wholePart, f64 decimalPart, bool negative) {

    bool expNegative = false;
    NBL_CHECK(context->reader.readAll(&context->lastRead, 1));
    if(context->lastRead == '-') {
        expNegative = true;
    } else if(context->lastRead != '+' && context->lastRead < '0' || context->lastRead > '9') {
        return nbl::Result::PARSE_ERROR;
    }

    u64 exponent = 0;
    while(true) {

        nbl::Result res = context->reader.readAll(&context->lastRead, 1);
        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead >= '0' && context->lastRead <= '9') {
                exponent *= 10;
                exponent += context->lastRead - '0';
                continue;
            }
            else if(_isWhitespace(context->lastRead)) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }

        if(decimalPart != 0.0) {
            f64 final = static_cast<f64>(wholePart) + decimalPart;
            if(negative) final = -final;

            for(usize i = 0 ; i < exponent ; i++) {
                final *= 10.0;
            }

            context->outElement = final;

        } else {

            for(usize i = 0 ; i < exponent ; i++) {
                wholePart *= 10;
            }

            if(negative) {
                context->outElement = -static_cast<i64>(wholePart);
            } else {
                context->outElement = wholePart;
            }
        }

        return nbl::Result::SUCCESS;
    }

}

nbl::Result _parseDecimalPart(_ParseContext* context, u64 wholePart, bool negative) {

    NBL_CHECK(context->reader.readAll(&context->lastRead, 1));
    if(context->lastRead < '0' || context->lastRead > '9') {
        return nbl::Result::PARSE_ERROR;
    }

    usize digits = 1;
    u64 decimalPart = context->lastRead - '0';
    
    while(true) {
        nbl::Result res = context->reader.readAll(&context->lastRead, 1);
        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead == 'e' || context->lastRead == 'E') {
                return _parseExponentPart(context, wholePart, static_cast<f64>(decimalPart) / (digits * 10), negative);
            }
            else if(context->lastRead >= '0' && context->lastRead <= '9') {
                decimalPart *= 10;
                decimalPart += context->lastRead - '0';
                digits++;
                continue;
            }
            else if(_isWhitespace(context->lastRead)) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }

        f64 after = static_cast<f64>(decimalPart);
        for(auto i = 0 ; i < digits ; i++) {
            after /= 10.0;
        }
        f64 final = static_cast<f64>(wholePart) + after;
        if(negative) final = -final;
        context->outElement = final;

        return nbl::Result::SUCCESS;
    }
}


nbl::Result _parseNumber(_ParseContext* context) {

    bool negative = false;
    if(context->lastRead == '-') {
        negative = true;
        NBL_CHECK(context->reader.readAll(&context->lastRead, 1));
    }

    if(context->lastRead == '0') {
        // Zero or decimal

        nbl::Result res = context->reader.readAll(&context->lastRead, 1);
        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead == '.') {
                return _parseDecimalPart(context, 0, negative);
            }
            else if(context->lastRead == 'e' || context->lastRead == 'E') {
                return _parseExponentPart(context, 0, 0.0, negative);
            }
            else if(_isWhitespace(context->lastRead)) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }
        context->outElement = nbl::JsonElement(0);
        return nbl::Result::SUCCESS;
    }

    if(context->lastRead < '1' || context->lastRead > '9') {
        return nbl::Result::PARSE_ERROR;
    }

    u64 wholePart = context->lastRead - '0';

    while(true) {
        nbl::Result res = context->reader.readAll(&context->lastRead, 1);

        if(res.error()) {
            if(res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
        } else {
            if(context->lastRead == '.') {
                return _parseDecimalPart(context, wholePart, negative);
            }
            else if(context->lastRead == 'e' || context->lastRead == 'E') {
                return _parseExponentPart(context, wholePart, 0.0, negative);
            }
            else if(context->lastRead >= '0' && context->lastRead <= '9') {
                wholePart *= 10;
                wholePart += context->lastRead - '0';
                continue;
            }
            else if(_isWhitespace(context->lastRead)) {
                res = _nextToken(context);
                if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            }
        }

        if(negative) {
            context->outElement = -static_cast<i64>(wholePart);
        } else {
            context->outElement = wholePart;
        }

        return nbl::Result::SUCCESS;
    }
}

nbl::Result _parseElement(_ParseContext* context) {

    char c = context->lastRead;
    switch(c) {
        case 'n': { // null
            u8 remaining[3];
            NBL_CHECK(context->reader.readAll(remaining, 3));
            if(memcmp(remaining, "ull", 3) == 0) {
                context->outElement = {};
            }
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            return nbl::Result::SUCCESS;
        }
        case 't': { // true
            u8 remaining[3];
            NBL_CHECK(context->reader.readAll(remaining, 3));
            if(memcmp(remaining, "rue", 3) == 0) {
                context->outElement = true;
            }
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            return nbl::Result::SUCCESS;
        }
        case 'f': { // false
            u8 remaining[4];
            NBL_CHECK(context->reader.readAll(remaining, 4));
            if(memcmp(remaining, "alse", 4) == 0) {
                context->outElement = false;
            }
            nbl::Result res = _nextToken(context);
            if(res.error() && res != nbl::Result::UNEXPECTED_END_OF_STREAM) return res;
            return nbl::Result::SUCCESS;
        }
        case '"': return _parseString(context);
        case '[': return _parseArray(context); 
        case '{': return _parseObject(context);
        default:
            if(c >= '0' && c <= '9' || c == '-') return _parseNumber(context);
            return nbl::Result::UNEXPECTED_TOKEN;
    }
}

}


nbl::Result nbl::parseJson(nbl::Reader reader, nbl::LinearAllocator* storage, nbl::JsonElement* outElement, nbl::LinearAllocator* tempStorage) {

    if(tempStorage == storage || storage == nullptr) return nbl::Result::INVALID_ARGUMENT;

    _ParseContext ctx = {
        .reader = reader,
        .storage = storage,
        .tempStorage = tempStorage,
        .outElement = {},
        .lastRead = 0
    };

    NBL_CHECK(_nextToken(&ctx));
    NBL_CHECK(_parseElement(&ctx));

    if(outElement) *outElement = ctx.outElement;
    return nbl::Result::SUCCESS;

}

nbl::Result nbl::parseJsonNumber(Reader reader, JsonNumber* outNumber) {

    _ParseContext ctx = {
        .reader = reader,
        .storage = nullptr,
        .tempStorage = nullptr,
        .outElement = {},
        .lastRead = 0
    };

    NBL_CHECK(_nextToken(&ctx));
    if(ctx.lastRead >= '0' && ctx.lastRead <= '9' || ctx.lastRead == '-') {
        NBL_CHECK(_parseNumber(&ctx));

        if(outNumber) *outNumber = ctx.outElement.number;
        return nbl::Result::SUCCESS;
    }

    return nbl::Result::UNEXPECTED_TOKEN;
}
