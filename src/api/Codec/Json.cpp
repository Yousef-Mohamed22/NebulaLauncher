// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Json.cpp
 * \author jwall64
 * \brief Implementation of JSON data structures and builders.
 */

#include <Core/Span.hpp>
#include <Core/Result.hpp>
#include <Platform/Logging.hpp>
#include <Codec/Json.hpp>


nbl::Result nbl::JsonArray::copyDataTo(ArenaFrame* storage, JsonArray* outArray) const {

    if(_mCount == 0) {
        outArray->_pValues = nullptr;
        outArray->_mCount = 0;
        return Result::SUCCESS;
    }

    JsonValue* ptr;
    NBL_CHECK(storage->allocate(&ptr, _mCount));

    for(usize i = 0 ; i < _mCount; i++) {
        const JsonValue& value = _pValues[i];
        ptr[i].type = value.type;
        if(value.type == JsonType::ARRAY) {
            NBL_CHECK(value.array.copyDataTo(storage, &ptr[i].array));
        } else if(value.type == JsonType::OBJECT) {
            NBL_CHECK(value.object.copyDataTo(storage, &ptr[i].object));
        } else if(value.type == JsonType::STRING) {
            char* strData;
            NBL_CHECK(storage->allocate(&strData, value.string.length));
            memcpy(strData, value.string.data, value.string.length);
            ptr[i].string = StringView(value.string.length, strData);
        } else {
            ptr[i] = value;
        }
    }

    outArray->_pValues = ptr;
    outArray->_mCount = _mCount;
    return Result::SUCCESS;
}

nbl::Result nbl::JsonArray::Builder::addChecked(nbl::JsonValue value) {

    if(_mLastResult.error()) {
        return _mLastResult;
    }

    nbl::JsonValue* ptr;
    _mLastResult = _mTemporaryStorage.allocate<nbl::JsonValue>(&ptr);
    if(_mLastResult.error()) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to add value to JsonArray builder: ", _mLastResult);
        return _mLastResult;
    }

    *ptr = value;
    _mCount++;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::JsonArray::Builder::build(nbl::ArenaFrame* storage, nbl::JsonArray* outArray) {

    if(_mCount == 0) {
        outArray->_pValues = nullptr;
        outArray->_mCount = 0;
        return nbl::Result::SUCCESS;
    }

    bool inPlace = (storage == nullptr);

    nbl::JsonValue* values;
    if(inPlace) {
        values = _pValues;
        storage = &_mTemporaryStorage;
    } else {
        NBL_CHECK(storage->allocate(&values, _mCount));
        for (usize i = 0; i < _mCount; i++) {
            values[i] = _pValues[i];
        }
    }

    for(usize i = 0; i < _mCount; i++) {
        const JsonValue& value = _pValues[i];
        if(value.type == JsonType::ARRAY) {
            NBL_CHECK(value.array.copyDataTo(storage, &values[i].array));
        } else if(value.type == JsonType::OBJECT) {
            NBL_CHECK(value.object.copyDataTo(storage, &values[i].object));
        } else if(value.type == JsonType::STRING) {
            char* strData;
            NBL_CHECK(storage->allocate(&strData, value.string.length));
            memcpy(strData, value.string.data, value.string.length);
            values[i].string = StringView(value.string.length, strData);
        }
    }

    if(inPlace) {
        _mTemporaryStorage.release();
    }

    outArray->_pValues = values;
    outArray->_mCount = _mCount;
    return nbl::Result::SUCCESS;
}

const nbl::JsonValue* nbl::JsonObject::get(StringView key) const {
    if (_mBucketCount == 0) return nullptr;

    usize hash = key.hash();
    usize index = hash % _mBucketCount;

    const _Bucket& bucket = _pBuckets[index];
    for (usize i = 0; i < bucket.count; i++) {
        const Entry& entry = _pEntries[bucket.offset + i];
        if (entry.key.length == key.length && memcmp(entry.key.data, key.data, key.length) == 0) {
            return &entry.value;
        }
    }
    return nullptr;
}

nbl::Result nbl::JsonObject::copyDataTo(ArenaFrame* storage, JsonObject* outObject) const {

    if(_mEntryCount == 0) {
        outObject->_pBuckets = nullptr;
        outObject->_mBucketCount = 0;
        outObject->_mEntryCount = 0;
        return Result::SUCCESS;
    }

    outObject->_mBucketCount = _mBucketCount;
    outObject->_mEntryCount = _mEntryCount;

    for(usize i = 0 ; i < _mEntryCount ; i++) {

        const Entry& entry = _pEntries[i];
        Entry& outEntry = outObject->_pEntries[i];

        char* keyData;
        NBL_CHECK(storage->allocate(&keyData, entry.key.length));
        memcpy(keyData, entry.key.data, entry.key.length);
        outEntry.key = StringView(entry.key.length, keyData);

        if(entry.value.type == JsonType::ARRAY) {
            NBL_CHECK(entry.value.array.copyDataTo(storage, &outEntry.value.array));
        } else if(entry.value.type == JsonType::OBJECT) {
            NBL_CHECK(entry.value.object.copyDataTo(storage, &outEntry.value.object));
        } else if(entry.value.type == JsonType::STRING) {
            char* strData;
            NBL_CHECK(storage->allocate(&strData, entry.value.string.length));
            memcpy(strData, entry.value.string.data, entry.value.string.length);
            outEntry.value.string = StringView(entry.value.string.length, strData);
        }
    }

    if(outObject != this) {

        NBL_CHECK(storage->allocate(&outObject->_pEntries, _mEntryCount));
        NBL_CHECK(storage->allocate(&outObject->_pBuckets, _mBucketCount));

        for(usize i = 0; i < _mBucketCount; i++) {
            outObject->_pBuckets[i] = _pBuckets[i];
        }

        for(usize i = 0 ; i < _mEntryCount ; i++) {
            outObject->_pEntries[i] = _pEntries[i];
        }
    }

    return Result::SUCCESS;
}

nbl::Result nbl::JsonObject::buildFromUnorderedEntries(nbl::Span<Entry> unorderedEntries, ArenaFrame* storage, JsonObject* outObject) {

    if(unorderedEntries.length == 0) {
        outObject->_pBuckets = nullptr;
        outObject->_pEntries = nullptr;
        outObject->_mBucketCount = 0;
        outObject->_mEntryCount = 0;
        return Result::SUCCESS;
    }

    outObject->_mEntryCount = unorderedEntries.length;
    outObject->_mBucketCount = unorderedEntries.length * 2;
    outObject->_pEntries = unorderedEntries.data;

    NBL_CHECK(storage->allocate(&outObject->_pBuckets, outObject->_mBucketCount));

    // Calculate bucket counts
    for(usize i = 0 ; i < unorderedEntries.length; i++) {
        const Entry& entry = unorderedEntries[i];

        usize hash = entry.key.hash();
        usize index = hash % outObject->_mBucketCount;

        outObject->_pBuckets[index].count++;
    }

    // Calculate bucket offsets
    usize currentOffset = 0;
    for(usize i = 0; i < outObject->_mBucketCount; i++) {
        _Bucket& bucket = outObject->_pBuckets[i];
        bucket.offset = currentOffset;
        currentOffset += bucket.count;
        bucket.count = 0;
    }

    // Reorder the entries
    {
        ArenaFrame frame(nbl::Arena::threadLocal());

        Entry* tempEntries;
        NBL_CHECK(frame.allocate(&tempEntries, outObject->_mEntryCount));

        for(usize i = 0 ; i < outObject->_mEntryCount; i++) {
            const Entry& entry = unorderedEntries[i];

            _Bucket& bucket = outObject->_pBuckets[entry.key.hash() % outObject->_mBucketCount];

            usize offset = bucket.offset + bucket.count;
            bucket.count++;

            tempEntries[offset] = entry;
        }

        // Copy back the entries
        for(usize i = 0; i < outObject->_mEntryCount; i++) {
            outObject->_pEntries[i] = tempEntries[i];
        }
    }

    return Result::SUCCESS;
}

nbl::Result nbl::JsonObject::Builder::addChecked(StringView key, JsonValue value) {

    if(_mLastResult.error()) {
        return _mLastResult;
    }

    Entry* ptr;
    _mLastResult = _mTemporaryStorage.allocate<Entry>(&ptr);
    if(_mLastResult.error()) {
        nbl::log(nbl::LogLevel::ERROR, "Failed to add entry to JsonObject builder: ", _mLastResult);
        return _mLastResult;
    }

    ptr->key = key;
    ptr->value = value;

    _mCount++;

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::JsonObject::Builder::build(ArenaFrame* storage, JsonObject* outObject) {

    if(_mLastResult.error()) {
        return _mLastResult;
    }

    if(_mCount == 0) {
        outObject->_pBuckets = nullptr;
        outObject->_pEntries = nullptr;
        outObject->_mBucketCount = 0;
        outObject->_mEntryCount = 0;
        return Result::SUCCESS;
    }

    outObject->_mEntryCount = _mCount;
    outObject->_mBucketCount = _mCount * 2;

    bool inPlace = (storage == nullptr);
    if(inPlace) { // Build in-place using temporary storage
        outObject->_pEntries = _pEntries;
        storage = &_mTemporaryStorage;        
    } else {
        NBL_CHECK(storage->allocate(&outObject->_pEntries, outObject->_mEntryCount));
        memcpy(outObject->_pEntries, _pEntries, sizeof(Entry) * _mCount);
    }


    // Copy entry data
    for(usize i = 0; i < outObject->_mEntryCount; i++) {

        const Entry& entry = _pEntries[i];
        Entry& newEntry = outObject->_pEntries[i];

        char* keyData;
        NBL_CHECK(storage->allocate(&keyData, nbl::align(entry.key.length, sizeof(void*))));
        memcpy(keyData, entry.key.data, entry.key.length);

        if(entry.value.type == JsonType::ARRAY) {

            NBL_CHECK(entry.value.array.copyDataTo(storage, &newEntry.value.array));

        } else if(entry.value.type == JsonType::OBJECT) {
 
            NBL_CHECK(entry.value.object.copyDataTo(storage, &newEntry.value.object));

        } else if(entry.value.type == JsonType::STRING) {

            char* strData;
            NBL_CHECK(storage->allocate(&strData, entry.value.string.length));
            memcpy(strData, entry.value.string.data, entry.value.string.length);
            
            newEntry.value.string = StringView(entry.value.string.length, strData);
        }
    }

    NBL_CHECK(buildFromUnorderedEntries({outObject->_pEntries, _mCount}, storage, outObject));

    if(inPlace) {
        _mTemporaryStorage.release();
    }

    return Result::SUCCESS;
}
