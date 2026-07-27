// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Json.hpp
 * \author jwall64
 * \brief A simple implementation of a JSON value type, along with array and object types for representing JSON data structures.
 */

#pragma once

#include <Core/Result.hpp>
#include <Core/Span.hpp>
#include <Core/Types.hpp>
#include <Core/String.hpp>

#include <Platform/StringBuilder.hpp>
#include <Platform/Arena.hpp>

namespace nbl {

/**
 * \brief An enumeration representing the different types of JSON values that can be represented. This includes basic types like boolean, number, and string, as well as complex types like arrays and objects. The NONE type represents an uninitialized or null value.
 */
enum class JsonType {
    NONE,
    BOOLEAN,
    NUMBER,
    STRING,
    ARRAY,
    OBJECT
};

/**
 * \brief An enumeration representing the different types of JSON numbers that can be represented. This includes floating-point numbers, signed integers, and unsigned integers. The type of a JSON number determines how it should be interpreted and serialized.
 */
enum class JsonNumberType {
    FLOAT,
    SIGNED,
    UNSIGNED
};

struct JsonValue;

/**
 * \brief A simple implementation of a JSON array type that can hold a sequence of JSON values.
 */
class JsonArray {
public:

    /** 
     * \brief Default constructor that creates an empty JSON array.
     */
    JsonArray() = default;

    JsonArray(const JsonValue* values, usize count) :
        _pValues(values),
        _mCount(count)
    { }

    /** 
     * \brief Index operator to access elements in the JSON array. .
     * \param index The index of the element to access.
     * \return A pointer to the JSON value at the specified index, or nullptr if the index is out of bounds.
     */
    const JsonValue* operator[](usize index) const { return get(index); }

    /** 
     * \brief Retrieves a pointer to the JSON value at the specified index in the array. 
     * \param index The index of the element to retrieve.
     * \return A pointer to the JSON value at the specified index, or nullptr if the index is out of bounds.
     */
    const JsonValue* get(usize index) const;

    /**  
     * \brief Returns the number of elements in the JSON array. .
     * \return The number of elements in the JSON array.
     */
    usize length() const { return _mCount; }

    /** 
     * \brief Returns a pointer to the underlying array of JSON values. 
     * \return A pointer to the underlying array of JSON values.
     */
    const JsonValue* data() const { return _pValues; }

    /** 
     * \brief Returns an iterator to the first element in the JSON array. 
     * \return A pointer to the first element in the JSON array.
     */
    const JsonValue* begin() const { return _pValues; }

    /** 
     * \brief Returns an iterator to one past the last element in the JSON array. 
     * \return A pointer to one past the last element in the JSON array.
     */
    const JsonValue* end() const;

    /** 
     * \brief Copies the data from this JSON array to another JSON array in a specified storage location. 
     * \param storage Pointer to the ArenaFrame where the copied data will be stored.
     * \param outArray Pointer to a variable that will receive the resulting JSON array after the copy operation is complete.
     * \return A Result indicating success or failure of the copy operation.
     */ 
    Result copyDataTo(ArenaFrame* storage, JsonArray* outArray) const;

    /** 
     * \brief A static constant representing an empty JSON array. This can be used as a default value or a placeholder for cases where an empty array is needed. The EMPTY array is initialized with no elements and can be safely used without modification.
     */
    static JsonArray EMPTY;

private:

    const JsonValue* _pValues;
    usize _mCount;

    friend class Builder;

public:

    /** 
     * \brief A builder class for constructing JSON arrays. 
     */
    class Builder {
    public:

        /** 
         * \brief Default constructor that initializes the builder with a thread-local ArenaFrame for temporary storage..
         */
        Builder() :
            _mLastResult(Result::SUCCESS),
            _mTemporaryStorage(Arena::threadLocal()),
            _pValues(static_cast<JsonValue*>(_mTemporaryStorage.current())),
            _mCount(0)
        { }

        /** 
         * \brief Constructor that initializes the builder with a specified ArenaFrame for temporary storage. 
         * \param temporaryStorage Pointer to the ArenaFrame that will be used for temporary storage during the construction of the JSON array.
         */
        explicit Builder(ArenaFrame* temporaryStorage) :
            _mLastResult(Result::SUCCESS),
            _mTemporaryStorage(temporaryStorage),
            _pValues(static_cast<JsonValue*>(_mTemporaryStorage.current())),
            _mCount(0)
        { }

        /** 
         * \brief Adds a JSON value to the builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param value The JSON value to be added to the current array being built.
         * \return A reference to the builder for chaining. If the add operation fails.
         */
        Builder& add(JsonValue value);
        
        /** 
         * \brief Adds a JSON value to the builder, with error checking. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param value The JSON value to be added to the current array being built.
         * \return A Result indicating success or failure of the add operation.
         */
        Result addChecked(JsonValue value);

        /** 
         * \brief Builds the final JSON array from the values that have been added to the builder into the specified frame.
         * \param storage Pointer to the ArenaFrame where the built JSON array will be stored. If this is nullptr, the JsonArray will be built in-place using the builder's temporary storage.
         * \param outArray Pointer to a variable that will receive the resulting JSON array after the build operation is complete. 
         * \return A Result indicating success or failure of the build operation.
         */
        Result build(ArenaFrame* storage, JsonArray* outArray);

    private:

        Result _mLastResult;
        ArenaFrame _mTemporaryStorage;
        JsonValue* _pValues;
        usize _mCount;
    };
};

/** 
 * \brief A simple implementation of a JSON object type that can hold a collection of key-value pairs, where the keys are strings and the values are JSON values.  */
class JsonObject {
public:    

    JsonObject() = default;

    /**
     * \brief Gets the JSON value associated with the specified key in the object. 
     * \param key The string key for which to retrieve the associated JSON value.
     * \return A pointer to the JSON value associated with the specified key, or nullptr if the key is not found in the object.
     */
    const JsonValue* get(StringView key) const;

    /** 
     * \brief Gets the JSON value associated with the specified key in the object, and returns it as an unsigned integer. 
     * \param key The string key for which to retrieve the associated JSON value.
     * \param defaultValue The default boolean value to return if the key is not found or if the associated value is not a boolean.
     * \return The boolean value associated with the specified key, or defaultValue if the key is not found or if the associated value is not a boolean.
     */
    u64 getUint(StringView key, u64 defaultValue = 0) const;

    /** 
     * \brief Gets the JSON value associated with the specified key in the object, and returns it as a signed integer.
     * \param key The string key for which to retrieve the associated JSON value.
     * \param defaultValue The default signed integer value to return if the key is not found or if the associated value is not a signed integer.
     * \return The signed integer value associated with the specified key, or defaultValue if the key is not found or if the associated value is not a signed integer.
     */
    i64 getInt(StringView key, i64 defaultValue = 0) const;

    /** 
     * \brief Gets the JSON value associated with the specified key in the object, and returns it as a floating-point number.
     * \param key The string key for which to retrieve the associated JSON value.
     * \param defaultValue The default floating-point value to return if the key is not found or if the associated value is not a number.
     * \return The floating-point value associated with the specified key, or defaultValue if the key is not found or if the associated value is not a number.
     */
    f64 getFloat(StringView key, f64 defaultValue = 0.0) const;

    /** 
     * \brief Creates a deep copy of the data from this JSON object.
     * \param storage Pointer to the ArenaFrame where the copied data will be stored.
     * \param outObject Pointer to a variable that will receive the resulting JSON object after the copy operation is complete.
     * \return A Result indicating success or failure of the copy operation. If there is not enough space in the arena to copy the data, it returns nbl::error::OUT_OF_MEMORY.
     */
    Result copyDataTo(ArenaFrame* storage, JsonObject* outObject) const;

    /** 
     * \brief Returns the number of key-value pairs in the JSON object. 
     * \return The number of key-value pairs in the JSON object.
     */
    usize count() const { return _mEntryCount; }

    /** 
     * \brief Represents a key-value pair entry in the JSON object. Each entry consists of a string key and a JSON value.  
     */
    struct Entry;

    /** 
     * \brief Returns an iterator to the first entry in the JSON object.
     * \return A pointer to the first entry in the JSON object.
     */
    const Entry* begin() const { return _pEntries; }

    /** 
     * \brief Returns an iterator to one past the last entry in the JSON object. 
     * \return A pointer to one past the last entry in the JSON object.
     */
    const Entry* end() const;


    static Result buildFromUnorderedEntries(nbl::Span<Entry> unorderedEntries, ArenaFrame* storage, JsonObject* outObject);

    static JsonObject EMPTY;

private:

    struct _Bucket {
        usize offset;
        usize count;
    };

    Entry* _pEntries;
    _Bucket* _pBuckets;
    usize _mBucketCount;
    usize _mEntryCount;

    friend class Builder;

public:

    /** 
     * \brief A builder class for constructing JSON objects.
     */
    class Builder {
    public:

        /** 
         * \brief Default constructor that initializes the builder with a thread-local ArenaFrame for temporary storage. 
         */
        Builder() :
            _mLastResult(Result::SUCCESS),
            _mTemporaryStorage(Arena::threadLocal()),
            _pEntries(static_cast<Entry*>(_mTemporaryStorage.current())),
            _mCount(0)
        { }

        /** 
         * \brief Constructor that initializes the builder with a specified ArenaFrame for temporary storage. 
         * \param temporaryStorage Pointer to the ArenaFrame that will be used for temporary storage during the construction of the JSON object.
         */
        explicit Builder(ArenaFrame* temporaryStorage) :
            _mLastResult(Result::SUCCESS),
            _mTemporaryStorage(temporaryStorage),
            _pEntries(static_cast<Entry*>(_mTemporaryStorage.current())),
            _mCount(0)
        { }

        /** 
         * \brief Adds a key-value pair to the builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, JsonValue value);

        /**
         * \brief Associates a string key with a string value in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, StringView value) { addChecked(key, value); return *this; }

        /**
         * \brief Associates a string key with an integer value in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, i64 value) { addChecked(key, value); return *this; }

        /**
         * \brief Associates a string key with an unsigned integer value in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, u64 value) { addChecked(key, value); return *this; }

        /**
         * \brief Associates a string key with a floating-point value in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, f64 value) { addChecked(key, value); return *this; }

        /**
         * \brief Associates a string key with a boolean value in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, bool value) { addChecked(key, value); return *this; }
      
        /**
         * \brief Associates a string key with an array in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, const JsonArray& value) { addChecked(key, value); return *this; }
     
        /**
         * \brief Associates a string key with an object in the JSON object being built. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The number value associated with the key to be added to the current object being built.
         * \return A reference to the builder for chaining.
         */
        Builder& add(StringView key, const JsonObject& value) { addChecked(key, value); return *this; }

        /** 
         * \brief Adds a key-value pair to the builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, JsonValue value);

        /** 
         * \brief Associates a string key with a string value in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, StringView value);

        /** 
         * \brief Associates a string key with an integer value in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, i64 value);

        /** 
         * \brief Associates a string key with an unsigned integer value in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, u64 value);

        /** 
         * \brief Associates a string key with a float value in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, f64 value);

        /** 
         * \brief Associates a string key with a boolean value in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, bool value);

        /** 
         * \brief Associates a string key with an array in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, const JsonArray& value);

        /** 
         * \brief Associates a string key with an object in the Builder. If the add operation fails, the builder will store the error result and all subsequent add operations will be no-ops.
         * \param key The string key for the entry to be added to the current object being built.
         * \param value The JSON value associated with the key to be added to the current object being built.
         * \return The result of the add operation.
         */
        Result addChecked(StringView key, const JsonObject& value);

        /** 
         * \brief Builds the final JSON object from the key-value pairs that have been added to the builder into the specified frame. 
         * \param storage Pointer to the ArenaFrame where the built JSON object will be stored. If this is nullptr, the JsonObject will be built in-place using the builder's temporary storage.
         * \param outObject Pointer to a variable that will receive the resulting JSON object after the build operation is complete. The outObject will point to the newly built JSON object in the provided storage location.
         * \return A Result indicating success or failure of the build operation.
         */
        Result build(ArenaFrame* storage, JsonObject* outObject);

    private:
        Result _mLastResult;
        ArenaFrame _mTemporaryStorage;
        Entry* _pEntries;
        usize _mCount;
    };
};

/** 
 * \brief A simple implementation of a JSON number type that can represent floating-point numbers, signed integers, and unsigned integers. 
 */
struct JsonNumber {

    /** 
     * \brief The type of the JSON number, which determines how the number should be interpreted and serialized. The type can be FLOAT for floating-point numbers, SIGNED for signed integers, or UNSIGNED for unsigned integers.
     */
    JsonNumberType type;

    /** 
     * \brief A union that holds the actual value of the JSON number.
     */
    union {
        f64 real;
        u64 uint;
        i64 sint;
    };

    /** 
     * \brief Converts the JSON number to a signed integer. The conversion is performed based on the type of the JSON number. If the number is a floating-point value, it will be truncated to an integer. If the number is an unsigned integer, it will be cast to a signed integer. If the number is already a signed integer, it will be returned as-is.
     * \return The JSON number converted to a signed integer.
     */
    i64 asInt() const { 
        switch(type) {
        case JsonNumberType::FLOAT:
            return static_cast<i64>(real);
        case JsonNumberType::UNSIGNED:
            return static_cast<i64>(uint);
        case JsonNumberType::SIGNED:
        default:
            return sint;
        }
    }

    /** 
     * \brief Converts the JSON number to an unsigned integer. The conversion is performed based on the type of the JSON number. If the number is a floating-point value, it will be truncated to an integer. If the number is a signed integer, it will be cast to an unsigned integer. If the number is already an unsigned integer, it will be returned as-is.
     * \return The JSON number converted to an unsigned integer.
     */
    u64 asUint() const { 
        switch(type) {
        case JsonNumberType::FLOAT:
            return static_cast<u64>(real);
        case JsonNumberType::UNSIGNED:
        default:
            return uint;
        case JsonNumberType::SIGNED:
            return static_cast<u64>(sint);
        }
    }

    /** 
     * \brief Converts the JSON number to a floating-point value. The conversion is performed based on the type of the JSON number. If the number is a signed or unsigned integer, it will be cast to a floating-point value. If the number is already a floating-point value, it will be returned as-is.
     * \return The JSON number converted to a floating-point value.
     */
    f64 asFloat() const { 
        switch(type) {
        case JsonNumberType::FLOAT:
        default:
            return real;
        case JsonNumberType::UNSIGNED:
            return static_cast<f64>(uint);
        case JsonNumberType::SIGNED:
            return static_cast<f64>(sint);
        }
    }

    /** 
     * \brief Equality operator to compare two JSON numbers for equality. The comparison is performed based on the type of the JSON numbers. If the types are different, the RHS is converted to the type of the LHS for comparison. If the types are the same, the values are compared directly.
     * \param other The other JSON number to compare with this JSON number.
     * \return true if the JSON numbers are considered equal, false otherwise.
     */
    bool operator==(const JsonNumber& other) const {
        switch (type) {
            case JsonNumberType::FLOAT:
                return real == other.asFloat();
            case JsonNumberType::UNSIGNED:
                return uint == other.asUint();
            case JsonNumberType::SIGNED:
                return sint == other.asInt();
        }
        return false;
    }

};

/** 
 * \brief A typed union that can represent any valid JSON value, including null, boolean, number, string, array, and object. The JsonValue struct uses a union to store the actual value of the JSON value, and it includes static methods for creating JSON values from different types of data.
 */
struct JsonValue {

    /** 
     * \brief The type of the JSON value, which determines how the value should be interpreted and serialized.
     */
    JsonType type;

    /** 
     * \brief A union that holds the actual value of the JSON value. The union can store a boolean, a JSON number, a string view, a JSON array, or a JSON object, depending on the type of the JSON value.
     */
    union {
        bool boolean;
        JsonNumber number;
        StringView string;
        JsonArray array;
        JsonObject object;
    };


    /** 
     * \brief Creates a JSON value from a signed integer.
     * \param value The signed integer value to be converted into a JSON value.
     * \return A JSON value representing the provided signed integer.
     */
    static JsonValue fromInt(i64 value) {
        return JsonValue { 
            .type = JsonType::NUMBER, 
            .number = { 
                .type = JsonNumberType::SIGNED, 
                .sint = value 
            }
        };
    }

    /** 
     * \brief Creates a JSON value from an unsigned integer.
     * \param value The signed integer value to be converted into a JSON value.
     * \return A JSON value representing the provided unsigned integer. 
     */
    static JsonValue fromUint(u64 value) {
        return JsonValue { 
            .type = JsonType::NUMBER, 
            .number = { 
                .type = JsonNumberType::UNSIGNED, 
                .uint = value 
            }
        };
    }

    /** 
     * \brief Creates a JSON value from a float.
     * \param value The signed integer value to be converted into a JSON value.
     * \return A JSON value representing the provided float.
     */
    static JsonValue fromFloat(f64 value) {
        return JsonValue { 
            .type = JsonType::NUMBER, 
            .number = { 
                .type = JsonNumberType::FLOAT, 
                .real = value 
            }
        };
    }
    
    /** 
     * \brief Creates a JSON value from a string view.
     * \param str The string view to be converted into a JSON value.
     * \return A JSON value representing the provided string view.
     */
    static JsonValue fromString(StringView str) {
        return JsonValue { .type = JsonType::STRING, .string = str };
    }

    /** 
     * \brief Creates a JSON value from a boolean value.
     * \param value The boolean value to be converted into a JSON value.
     * \return A JSON value representing the provided boolean value.
     */
    static JsonValue fromBoolean(bool value) {
        return value ? TRUE_VALUE : FALSE_VALUE;
    }

    /** 
     * \brief Creates a JSON value from a JSON array.
     * \param array The JSON array to be converted into a JSON value.
     * \return A JSON value representing the provided JSON array.
     */
    static JsonValue fromArray(const JsonArray& array) {
        return JsonValue { .type = JsonType::ARRAY, .array = array };
    }

    /** 
     * \brief Creates a JSON value from a JSON object.
     * \param object The JSON object to be converted into a JSON value.
     * \return A JSON value representing the provided JSON object.
     */
    static JsonValue fromObject(const JsonObject& object) {
        return JsonValue { .type = JsonType::OBJECT, .object = object };
    }

    /**
     * \brief A static constant representing the JSON boolean value true.
     */
    static JsonValue TRUE_VALUE;

    /**
     * \brief A static constant representing the JSON boolean value false. 
     */
    static JsonValue FALSE_VALUE;

    /**
     * \brief A static constant representing the JSON null value.
     */
    static JsonValue NULL_VALUE;

};


/** 
 * \brief Parses a JSON value from a reader and stores it in the provided storage location.  
 * \param reader The Reader from which to read the JSON data.
 * \param storage Pointer to the ArenaFrame where the parsed JSON value will be stored.
 * \param outValue Pointer to a variable that will receive the resulting JSON value after the parse operation is complete.
 * \return A Result indicating success or failure of the parse operation. 
 */
Result parseJson(Reader reader, ArenaFrame* storage, JsonValue* outValue, bool inPlace = false);

/**
 * \brief Parses a JSON number from a reader and stores it in the provided outNumber parameter. 
 * \param reader The Reader from which to read the JSON number data.
 * \param outNumber Pointer to a variable that will receive the resulting JSON number after the parse operation is complete
 * \return A Result indicating success or failure of the parse operation.
 */
Result parseJsonNumber(Reader reader, JsonNumber* outNumber);


/** 
 * \brief Writes a JSON value to a writer. 
 * \param writer The Writer to which the JSON data will be written.
 * \param value The JsonValue that represents the JSON value to be serialized and written to the writer.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeJson(Writer writer, const JsonValue& value);

/** 
 * \brief Writes a JSON array to a writer
 * \param writer The Writer to which the JSON data will be written.
 * \param value The JsonArray that represents the JSON array to be serialized and written to the writer.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeJsonArray(Writer writer, const JsonArray& value);

/** 
 * \brief Writes a JSON object to a writer.
 * \param writer The Writer to which the JSON data will be written.
 * \param value The JsonArray that represents the JSON object to be serialized and written to the writer.
 * \return A Result indicating success or failure of the write operation.
 */
Result writeJsonObject(Writer writer, const JsonObject& value);


/** 
 * \brief A class for parsing JSON data from a Reader in a manual fashion. The PartialJsonParser allows for incremental parsing of JSON data, which can be useful for handling large or incomplete JSON documents.
 */
class PartialJsonParser {
public:

    /** 
     * \brief Constructor that initializes the parser with a Reader from which to read the JSON data. 
     * \param reader The Reader from which to read the JSON data.
     */
    PartialJsonParser(Reader reader) :
        _mReader(reader),
        _mLastResult(Result::SUCCESS),
        _mCurrentChar(0)
    { }

    /**
     * \brief Consumes a '{' from the input stream, indicating the beginning of a JSON object. 
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& beginObject();

    /** 
     * \brief Consumes a '}' from the input stream, indicating the end of a JSON object. 
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& endObject();

    /** 
     * \brief Consumes a '[' from the input stream, indicating the beginning of a JSON array. 
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& beginArray();

    /** 
     * \brief Consumes a ']' from the input stream, indicating the end of a JSON array. 
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& endArray();

    /** 
     * \brief Reads a key from the input stream, and consumes the following ':' character.
     * \param outKey Pointer to a StringBuilder that will receive the key read from the input stream.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& readKey(StringBuilder* outKey);

    PartialJsonParser& expectKey(StringView expectedKey);

    /**
     * \brief Reads any JSON value from the input stream, and stores it in the provided storage location.
     * \param frame Pointer to the ArenaFrame where the parsed JSON value will be stored. 
     * \param outValue Pointer to a variable that will receive the resulting JSON value after the parse operation is complete.
     */
    PartialJsonParser& readValue(ArenaFrame* frame, JsonValue* outValue);

    /** 
     * \brief Reads a JSON object from the input stream, and stores it in the provided storage location. 
     * \param frame Pointer to the ArenaFrame where the parsed JSON object will be stored. 
     * \param outObject Pointer to a variable that will receive the resulting JSON object after the parse operation is complete.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    inline PartialJsonParser& readObject(ArenaFrame* frame, JsonObject* outObject) {
        JsonValue value;
        readValue(frame, &value);
        if(_mLastResult.error()) return *this; 
        if(value.type != JsonType::OBJECT) {
            _mLastResult = nbl::error::UNEXPECTED_TOKEN;
            return *this;
        } 
        *outObject = value.object;
        return *this;
    }

    /** 
     * \brief Reads a JSON array from the input stream, and stores it in the provided storage location. 
     * \param frame Pointer to the ArenaFrame where the parsed JSON array will be stored. 
     * \param outArray Pointer to a variable that will receive the resulting JSON array after the parse operation is complete.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& readArray(ArenaFrame* frame, JsonArray* outArray) {
        JsonValue value;
        readValue(frame, &value);
        if(_mLastResult.error()) return *this; 
        if(value.type != JsonType::ARRAY) {
            _mLastResult = nbl::error::UNEXPECTED_TOKEN;
            return *this;
        } 
        *outArray = value.array;
        return *this;
    }

    /**
     * \brief Reads a JSON string from the input stream, and stores it in the provided StringBuilder. 
     * \param outString Pointer to a StringBuilder that will receive the resulting JSON string after the parse operation is complete.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& readString(StringBuilder* outString);

    /** 
     * \brief Reads a JSON number from the input stream, and stores it in the provided JsonNumber variable. 
     * \param outNumber Pointer to a variable that will receive the resulting JSON number after the parse operation is complete.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& readNumber(JsonNumber* outNumber);

    /** 
     * \brief Reads a JSON boolean value from the input stream, and stores it in the provided boolean variable. 
     * \param outBoolean Pointer to a variable that will receive the resulting JSON boolean value after the parse operation is complete.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& readBoolean(bool* outBoolean);

    /** 
     * \brief Reads a JSON number from the input stream, converts it to an unsigned integer, and stores it in the provided variable.
     * \param outUint Pointer to a variable that will receive the resulting unsigned integer value after the parse operation is complete.  
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    template<typename T>
    inline PartialJsonParser& readUint(T* outUint) {
        JsonNumber number;
        readNumber(&number);
        if(!_mLastResult.error()) *outUint = static_cast<T>(number.asUint());
        return *this;
    }
    
    /** 
     * \brief Reads a JSON number from the input stream, converts it to an integer, and stores it in the provided variable.
     * \param outInt Pointer to a variable that will receive the resulting signed integer value after the parse operation is complete.  
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    template<typename T>
    inline PartialJsonParser& readInt(T* outInt) {
        JsonNumber number;
        readNumber(&number);
        if(!_mLastResult.error()) *outInt = static_cast<T>(number.asInt());
        return *this;
    }
    
    /** 
     * \brief Reads a JSON number from the input stream, converts it to a float, and stores it in the provided variable.
     * \param outFloat Pointer to a variable that will receive the resulting float value after the parse operation is complete.  
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    template<typename T>
    inline PartialJsonParser& readFloat(T* outFloat) { 
        JsonNumber number;
        readNumber(&number);
        if(!_mLastResult.error()) *outFloat = static_cast<T>(number.asFloat());
        return *this;
    }

    /** 
     * \brief Reads a JSON null value from the input stream. If the next value in the input stream is not null, the parse operation will fail and the parser will store the error result.
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& readNull();

    /** 
     * \brief Skips a key in the input stream, consuming the key itself and the following ':' character. 
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& skipKey();

    /** 
     * \brief Skips any JSON value in the input stream, consuming the entire value including any nested structures. 
     * \return A reference to the parser for chaining. If the parse operation fails, the parser will store the error result and all subsequent parse operations will be no-ops.
     */
    PartialJsonParser& skipValue();

    /** 
     * \brief Gets the result of the last parse operation performed by the parser. 
     * \return A Result indicating the success or failure of the last parse operation.  
     */
    nbl::Result result() const { return _mLastResult; }

private:

    nbl::Reader _mReader;
    nbl::Result _mLastResult;
    char _mCurrentChar;

};


// toString overloads. See String.hpp
inline Result toString(const JsonValue& value, Writer writer) {
    return writeJson(writer, value);
}

inline Result toString(const JsonArray& value, Writer writer) {
    return writeJsonArray(writer, value);
}

inline Result toString(const JsonObject& value, Writer writer) {
    return writeJsonObject(writer, value);
}


// Inline implementations
inline const JsonValue* JsonArray::get(usize index) const {
    if (index >= _mCount) return nullptr;
    return &_pValues[index];
}

inline const JsonValue* JsonArray::end() const {
    return _pValues + _mCount;
}

inline JsonArray JsonArray::EMPTY(nullptr, 0);
inline JsonObject JsonObject::EMPTY;

inline JsonArray::Builder& JsonArray::Builder::add(JsonValue value) {
    addChecked(value);
    return *this;
}

struct JsonObject::Entry {
    StringView key;
    JsonValue value;
};



inline const JsonObject::Entry* JsonObject::end() const {
    return _pEntries + _mEntryCount;
}

inline JsonObject::Builder& JsonObject::Builder::add(StringView key, JsonValue value) {
    addChecked(key, value);
    return *this;
}

inline Result JsonObject::Builder::addChecked(StringView key, StringView value) {
    return addChecked(key, nbl::JsonValue::fromString(value));
}
inline Result JsonObject::Builder::addChecked(StringView key, i64 value) {
    return addChecked(key, nbl::JsonValue::fromInt(value));
}
inline Result JsonObject::Builder::addChecked(StringView key, u64 value) {
    return addChecked(key, nbl::JsonValue::fromUint(value));
}
inline Result JsonObject::Builder::addChecked(StringView key, f64 value) {
    return addChecked(key, nbl::JsonValue::fromFloat(value));
}
inline Result JsonObject::Builder::addChecked(StringView key, bool value) {
    return addChecked(key, value ? nbl::JsonValue::TRUE_VALUE : nbl::JsonValue::FALSE_VALUE);
}
inline Result JsonObject::Builder::addChecked(StringView key, const JsonArray& value) {
    return addChecked(key, nbl::JsonValue { .type = JsonType::ARRAY, .array = value });
}
inline Result JsonObject::Builder::addChecked(StringView key, const JsonObject& value) {
    return addChecked(key, nbl::JsonValue { .type = JsonType::OBJECT, .object = value });
}


inline u64 nbl::JsonObject::getUint(StringView key, u64 defaultValue) const {
    const JsonValue* value = get(key);
    if (!value || value->type != JsonType::NUMBER) {
        return defaultValue;
    }
    return value->number.asUint();
}

inline i64 nbl::JsonObject::getInt(StringView key, i64 defaultValue) const {
    const JsonValue* value = get(key);
    if (!value || value->type != JsonType::NUMBER) {
        return defaultValue;
    }
    return value->number.asInt();
}

inline f64 nbl::JsonObject::getFloat(StringView key, f64 defaultValue) const {
    const JsonValue* value = get(key);
    if (!value || value->type != JsonType::NUMBER) {
        return defaultValue;
    }
    return value->number.asFloat();
}


inline JsonValue JsonValue::TRUE_VALUE       = { .type = JsonType::BOOLEAN, .boolean = true };
inline JsonValue JsonValue::FALSE_VALUE      = { .type = JsonType::BOOLEAN, .boolean = false };
inline JsonValue JsonValue::NULL_VALUE = { .type = JsonType::NONE };

}
