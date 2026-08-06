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


#include <gtest/gtest.h>

#include <Platform/Logging.hpp>
#include <Codec/Json.hpp>
#include <Core/Stream.hpp>

TEST(Json, ParseNull) {
    nbl::JsonElement element = {};
    nbl::StringView json = "null";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NONE);
}

TEST(Json, ParseTrue) {
    nbl::JsonElement element = {};
    nbl::StringView json = "true";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(element.boolean, true);
}

TEST(Json, ParseFalse) {
    nbl::JsonElement element = {};
    nbl::StringView json = "false";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(element.boolean, false);
}

TEST(Json, ParseInt) {
    nbl::JsonElement element = {};
    nbl::StringView json = "-145";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::SIGNED);
    EXPECT_EQ(element.number.intValue, -145);
}

TEST(Json, ParseUint) {
    nbl::JsonElement element = {};
    nbl::StringView json = "145";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_NE(element.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(element.number.uintValue, 145);
}

TEST(Json, ParseZero) {
    nbl::JsonElement element = {};
    nbl::StringView json = "0";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::SIGNED);
    EXPECT_EQ(element.number.intValue, 0);
    EXPECT_EQ(element.number.asUint(), 0);
    EXPECT_EQ(element.number.asFloat(), 0.0);
}

TEST(Json, ParseNegativeZero) {
    nbl::JsonElement element = {};
    nbl::StringView json = "-0";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::SIGNED);
    EXPECT_EQ(element.number.intValue, 0);
}

TEST(Json, ParseFloat) {
    nbl::JsonElement element = {};
    nbl::StringView json = "1.45";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(element.number.floatValue, 1.45);
}

TEST(Json, ParseNegativeFloat) {
    nbl::JsonElement element = {};
    nbl::StringView json = "-1.5";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(element.number.asFloat(), -1.5);
    EXPECT_EQ(element.number.asInt(), -1);
}

TEST(Json, ParseExponentPositive) {
    nbl::JsonElement element = {};
    nbl::StringView json = "1e10";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(element.number.uintValue, 10000000000ULL);
}

TEST(Json, ParseExponentNegative) {
    nbl::JsonElement element = {};
    nbl::StringView json = "1.5e-3";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_NEAR(element.number.floatValue, 0.0015, 1e-9);
}

TEST(Json, ParseExponentUppercase) {
    nbl::JsonElement element = {};
    nbl::StringView json = "1E10";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.uintValue, 10000000000ULL);
}

TEST(Json, ParseLargeUint) {
    nbl::JsonElement element = {};
    nbl::StringView json = "18446744073709551615";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(element.number.uintValue, 18446744073709551615ULL);
}

TEST(Json, ParseString) {
    nbl::JsonElement element = {};
    nbl::StringView json = "\"Hello, World\"";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string, nbl::StringView("Hello, World"));
}

TEST(Json, ParseEmptyString) {
    nbl::JsonElement element = {};
    nbl::StringView json = "\"\"";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string.length, 0u);
    EXPECT_STREQ(element.string.data, "");
}

TEST(Json, ParseStringWithEscapes) {
    nbl::JsonElement element = {};
    // \" \\ \/ \b \f \n \r \t
    nbl::StringView json = "\"quote \\\\ slash\\/ back\\b form\\n new\\r cari\\t tab\"";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string, nbl::StringView("quote \\ slash/ back\b form\n new\r cari\t tab"));
}

TEST(Json, ParseStringUnicodeEscape) {
    nbl::JsonElement element = {};
    nbl::StringView json = "\"\\u0041\\u0042\\u0043\""; // "ABC"
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string, nbl::StringView("ABC"));
}

TEST(Json, ParseStringUnicodeSurrogatePair) {
    nbl::JsonElement element = {};
    // U+1F600 😀 encoded as UTF-16 surrogate pair \uD83D\uDE00
    nbl::StringView json = "\"\\uD83D\\uDE00\"";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    // 😀 is 4 bytes in UTF-8: F0 9F 98 80
    EXPECT_EQ(element.string.length, 4u);
    EXPECT_EQ(static_cast<unsigned char>(element.string.data[0]), 0xF0);
    EXPECT_EQ(static_cast<unsigned char>(element.string.data[1]), 0x9F);
    EXPECT_EQ(static_cast<unsigned char>(element.string.data[2]), 0x98);
    EXPECT_EQ(static_cast<unsigned char>(element.string.data[3]), 0x80);
}

TEST(Json, ParseArray) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[1,2,3]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 3);
    EXPECT_EQ(element.array[0].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[0].number.asUint(), 1);
    EXPECT_EQ(element.array[1].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[1].number.asUint(), 2);
    EXPECT_EQ(element.array[2].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[2].number.asUint(), 3);
}

TEST(Json, ParseEmptyArray) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 0);
    EXPECT_EQ(element.array.elements, nullptr);
}

TEST(Json, ParseSingleElementArray) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[42]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 1);
    EXPECT_EQ(element.array[0].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[0].number.asUint(), 42);
}

TEST(Json, ParseHeterogeneousArray) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[1,\"two\",true,null,[2]]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 5);
    EXPECT_EQ(element.array[0].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[0].number.asUint(), 1);
    EXPECT_EQ(element.array[1].type, nbl::JsonType::STRING);
    EXPECT_EQ(element.array[1].string, "two");
    EXPECT_EQ(element.array[2].type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(element.array[2].boolean, true);
    EXPECT_EQ(element.array[3].type, nbl::JsonType::NONE);
    EXPECT_EQ(element.array[4].type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array[4].array.count, 1);
    EXPECT_EQ(element.array[4].array[0].number.asUint(), 2);
}

TEST(Json, ParseNestedArray) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[[1,2],[3,4]]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 2);
    EXPECT_EQ(element.array[0].type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array[0].array.count, 2);
    EXPECT_EQ(element.array[0].array[0].number.asUint(), 1);
    EXPECT_EQ(element.array[0].array[1].number.asUint(), 2);
    EXPECT_EQ(element.array[1].type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array[1].array.count, 2);
    EXPECT_EQ(element.array[1].array[0].number.asUint(), 3);
    EXPECT_EQ(element.array[1].array[1].number.asUint(), 4);
}

TEST(Json, ParseArrayOutOfBounds) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[1,2,3]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.array.get(3), nullptr);
}

TEST(Json, ParseLargeArray) {
    nbl::JsonElement element = {};
    std::string json = "[";
    for (int i = 0; i < 200; ++i) {
        if (i != 0) json += ",";
        json += std::to_string(i);
    }
    json += "]";

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(4096, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(nbl::StringView(json.data(), json.size()), &storage, &element),
            nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 200);
    EXPECT_EQ(element.array[0].number.asUint(), 0);
    EXPECT_EQ(element.array[199].number.asUint(), 199);
}

TEST(Json, ParseArrayIteration) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[10,20,30]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);

    usize count = 0;
    u64 sum = 0;
    for (auto it = element.array.begin(); it != element.array.end(); ++it) {
        EXPECT_EQ(it->type, nbl::JsonType::NUMBER);
        sum += it->number.asUint();
        ++count;
    }
    EXPECT_EQ(count, 3u);
    EXPECT_EQ(sum, 60u);
}

TEST(Json, ParseObject) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"key\":99}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 1);

    const nbl::JsonElement* e = element.object.get("key");
    EXPECT_NE(e, nullptr);
    EXPECT_EQ(e->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(e->number.asUint(), 99);
}

TEST(Json, ParseEmptyObject) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 0);
    EXPECT_EQ(element.object.entries, nullptr);
    EXPECT_FALSE(element.object.has("anything"));
    EXPECT_EQ(element.object.get("missing"), nullptr);
}

TEST(Json, ParseSubObject) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"sub\":{\"key\":\"value\"}}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 1);

    const nbl::JsonElement* e = element.object.get("sub");
    EXPECT_NE(e, nullptr);
    EXPECT_EQ(e->type, nbl::JsonType::OBJECT);
    EXPECT_EQ(e->object.entryCount, 1);

    const nbl::JsonElement* e2 = e->object.get("key");
    EXPECT_NE(e2, nullptr);
    EXPECT_EQ(e2->type, nbl::JsonType::STRING);
    EXPECT_EQ(e2->string, "value");
}

TEST(Json, ParseComplex) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"int\":0,\"float\":99.5,\"str\":\"TEST\",\"bool\":true,\"obj\":{\"key\":\"value\"},\"arr\":[1,2,3]}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 6);

    const nbl::JsonElement* eInt = element.object.get("int");
    const nbl::JsonElement* eFloat = element.object.get("float");
    const nbl::JsonElement* eStr = element.object.get("str");
    const nbl::JsonElement* eBool = element.object.get("bool");
    const nbl::JsonElement* eObj = element.object.get("obj");
    const nbl::JsonElement* eArr = element.object.get("arr");

    EXPECT_NE(eInt, nullptr);
    EXPECT_EQ(eInt->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eInt->number.asInt(), 0);

    EXPECT_NE(eFloat, nullptr);
    EXPECT_EQ(eFloat->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eFloat->number.asFloat(), 99.5);

    EXPECT_NE(eStr, nullptr);
    EXPECT_EQ(eStr->type, nbl::JsonType::STRING);
    EXPECT_EQ(eStr->string, "TEST");

    EXPECT_NE(eBool, nullptr);
    EXPECT_EQ(eBool->type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(eBool->boolean, true);

    EXPECT_NE(eObj, nullptr);
    EXPECT_EQ(eObj->type, nbl::JsonType::OBJECT);
    EXPECT_EQ(eObj->object.entryCount, 1);
    EXPECT_TRUE(eObj->object.has("key"));
    EXPECT_EQ(eObj->object.get("key")->type, nbl::JsonType::STRING);
    EXPECT_EQ(eObj->object.get("key")->string, "value");

    EXPECT_EQ(eArr->type, nbl::JsonType::ARRAY);
    EXPECT_EQ(eArr->array.count, 3);
    EXPECT_EQ(eArr->array[0].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eArr->array[0].number.asUint(), 1);
    EXPECT_EQ(eArr->array[1].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eArr->array[1].number.asUint(), 2);
    EXPECT_EQ(eArr->array[2].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eArr->array[2].number.asUint(), 3);
}

TEST(Json, ParseObjectMultipleEntries) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"a\":1,\"b\":2,\"c\":3}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 3);

    EXPECT_TRUE(element.object.has("a"));
    EXPECT_TRUE(element.object.has("b"));
    EXPECT_TRUE(element.object.has("c"));
    EXPECT_FALSE(element.object.has("d"));

    EXPECT_EQ(element.object.get("a")->number.asUint(), 1);
    EXPECT_EQ(element.object.get("b")->number.asUint(), 2);
    EXPECT_EQ(element.object.get("c")->number.asUint(), 3);
}

TEST(Json, ParseDeepNesting) {
    nbl::JsonElement element = {};
    std::string json = "{\"v\":42}";
    // Wrap in 50 nested objects
    for (int i = 0; i < 50; ++i) {
        json = "{\"o\":" + json + "}";
    }

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(65536, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(nbl::StringView(json.data(), json.size()), &storage, &element),
            nbl::Result::SUCCESS);

    const nbl::JsonElement* cur = &element;
    for (int i = 0; i < 50; ++i) {
        ASSERT_NE(cur, nullptr);
        EXPECT_EQ(cur->type, nbl::JsonType::OBJECT);
        cur = cur->object.get("o");
    }
    ASSERT_NE(cur, nullptr);
    cur = cur->object.get("v");
    ASSERT_NE(cur, nullptr);
    EXPECT_EQ(cur->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(cur->number.asUint(), 42);
}

TEST(Json, ParseObjectObjectDefaultHelpers) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"num\":7,\"str\":\"hi\",\"bool\":false,\"obj\":{\"x\":1},\"arr\":[1,2]}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.object.getNumber("num", nbl::JsonNumber { .type = nbl::JsonNumberType::SIGNED, .intValue = -1 }).asInt(), 7);
    EXPECT_EQ(element.object.getNumber("missing", nbl::JsonNumber { .type = nbl::JsonNumberType::SIGNED, .intValue = -1 }).asInt(), -1);
    EXPECT_EQ(element.object.getString("str", "default").data[0], 'h');
    EXPECT_EQ(element.object.getString("missing", "default"), nbl::StringView("default"));
    EXPECT_EQ(element.object.getBoolean("bool", true), false);
    EXPECT_EQ(element.object.getBoolean("missing", true), true);
    EXPECT_EQ(element.object.getObject("obj", {}).entryCount, 1);
    EXPECT_EQ(element.object.getObject("missing", {}).entryCount, 0);
    EXPECT_EQ(element.object.getArray("arr", {}).count, 2);
    EXPECT_EQ(element.object.getArray("missing", {}).count, 0);
}

TEST(Json, ParseArrayObjectDefaultHelpers) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[{\"name\":\"a\"},{\"name\":\"b\"},42,\"str\",true]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);

    EXPECT_EQ(element.array.getObject(0, {}).getString("name", "").data[0], 'a');
    EXPECT_EQ(element.array.getObject(99, {}).entryCount, 0);
    EXPECT_EQ(element.array.getArray(99, {}).count, 0);
    EXPECT_EQ(element.array.getString(3, "def"), nbl::StringView("str"));
    EXPECT_EQ(element.array.getString(99, "def"), nbl::StringView("def"));
    EXPECT_EQ(element.array.getNumber(2, nbl::JsonNumber { .type = nbl::JsonNumberType::SIGNED, .intValue = -1 }).asInt(), 42);
    EXPECT_EQ(element.array.getNumber(99, nbl::JsonNumber { .type = nbl::JsonNumberType::SIGNED, .intValue = -1 }).asInt(), -1);
    EXPECT_EQ(element.array.getBoolean(4, false), true);
    EXPECT_EQ(element.array.getBoolean(99, false), false);
}

TEST(Json, ParseObjectIteration) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"a\":1,\"b\":2}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);

    usize count = 0;
    for (auto it = element.object.begin(); it != element.object.end(); ++it) {
        EXPECT_EQ(it->value.type, nbl::JsonType::NUMBER);
        ++count;
    }
    EXPECT_EQ(count, 2u);
}

TEST(Json, ParseLargeObject) {
    nbl::JsonElement element = {};
    std::string json = "{";
    for (int i = 0; i < 200; ++i) {
        if (i != 0) json += ",";
        json += "\"k" + std::to_string(i) + "\":" + std::to_string(i);
    }
    json += "}";

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(65536, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(nbl::StringView(json.data(), json.size()), &storage, &element),
            nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 200);
    EXPECT_NE(element.object.get("k199"), nullptr);
    EXPECT_EQ(element.object.get("k199")->number.asUint(), 199);
    EXPECT_EQ(element.object.get("k0")->number.asUint(), 0);
}

TEST(Json, ParseComplexFromStream) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"int\":0,\"float\":99.5,\"str\":\"TEST\",\"bool\":true,\"obj\":{\"key\":\"value\"},\"arr\":[1,2,3]}";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 6);

    const nbl::JsonElement* eInt = element.object.get("int");
    const nbl::JsonElement* eFloat = element.object.get("float");
    const nbl::JsonElement* eStr = element.object.get("str");
    const nbl::JsonElement* eBool = element.object.get("bool");
    const nbl::JsonElement* eObj = element.object.get("obj");
    const nbl::JsonElement* eArr = element.object.get("arr");

    EXPECT_NE(eInt, nullptr);
    EXPECT_EQ(eInt->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eInt->number.asInt(), 0);

    EXPECT_NE(eFloat, nullptr);
    EXPECT_EQ(eFloat->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eFloat->number.asFloat(), 99.5);

    EXPECT_NE(eStr, nullptr);
    EXPECT_EQ(eStr->type, nbl::JsonType::STRING);
    EXPECT_EQ(eStr->string, "TEST");

    EXPECT_NE(eBool, nullptr);
    EXPECT_EQ(eBool->type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(eBool->boolean, true);

    EXPECT_NE(eObj, nullptr);
    EXPECT_EQ(eObj->type, nbl::JsonType::OBJECT);
    EXPECT_EQ(eObj->object.entryCount, 1);
    EXPECT_TRUE(eObj->object.has("key"));
    EXPECT_EQ(eObj->object.get("key")->type, nbl::JsonType::STRING);
    EXPECT_EQ(eObj->object.get("key")->string, "value");

    EXPECT_EQ(eArr->type, nbl::JsonType::ARRAY);
    EXPECT_EQ(eArr->array.count, 3);
    EXPECT_EQ(eArr->array[0].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eArr->array[0].number.asUint(), 1);
    EXPECT_EQ(eArr->array[1].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eArr->array[1].number.asUint(), 2);
    EXPECT_EQ(eArr->array[2].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(eArr->array[2].number.asUint(), 3);
}

TEST(Json, ParseFromStreamWithTempStorage) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"key\":42}";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    nbl::Arena temp;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::Arena::create(1024, &temp), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element, &temp), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 1);
    const nbl::JsonElement* e = element.object.get("key");
    EXPECT_NE(e, nullptr);
    EXPECT_EQ(e->number.asUint(), 42);
}

TEST(Json, ParseInvalidArgumentWhenTempEqualsStorage) {
    nbl::JsonElement element = {};
    nbl::StringView json = "null";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element, &storage), nbl::Result::INVALID_ARGUMENT);
}

TEST(Json, ParseInvalidArgumentWhenStorageNull) {
    nbl::JsonElement element = {};
    nbl::StringView json = "null";
    EXPECT_EQ(nbl::parseJson(json, nullptr, &element), nbl::Result::INVALID_ARGUMENT);
}

TEST(Json, ParseErrorEmptyString) {
    nbl::JsonElement element = {};
    nbl::StringView json = "";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::UNEXPECTED_END_OF_STREAM);
}

TEST(Json, ParseErrorWhitespaceOnly) {
    nbl::JsonElement element = {};
    nbl::StringView json = "   \n\t ";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::UNEXPECTED_END_OF_STREAM);
}

TEST(Json, ParseErrorInvalidLiteralN) {
    nbl::JsonElement element = {};
    nbl::StringView json = "n";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidLiteralNu) {
    nbl::JsonElement element = {};
    nbl::StringView json = "nu";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidLiteralNul) {
    nbl::JsonElement element = {};
    nbl::StringView json = "nul";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidLiteralTru) {
    nbl::JsonElement element = {};
    nbl::StringView json = "tru";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidLiteralFalsy) {
    nbl::JsonElement element = {};
    nbl::StringView json = "falsy";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidToken) {
    nbl::JsonElement element = {};
    nbl::StringView json = "garbage";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorUnterminatedString) {
    nbl::JsonElement element = {};
    nbl::StringView json = "\"unterminated";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidEscape) {
    nbl::JsonElement element = {};
    nbl::StringView json = "\"\\q\"";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorMissingColon) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"key\" 42}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorMissingComma) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"a\":1 \"b\":2}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorMismatchedBracketsArray) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[1}";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorMismatchedBracketsObject) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{1]";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidNumberDotStart) {
    nbl::JsonElement element = {};
    nbl::StringView json = ".5";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidNumberDoubleDot) {
    nbl::JsonElement element = {};
    nbl::StringView json = "1..2";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorInvalidNumberExponentNoDigits) {
    nbl::JsonElement element = {};
    nbl::StringView json = "1e";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorLeadingZero) {
    nbl::JsonElement element = {};
    nbl::StringView json = "01";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorEmptyObjectMissingClose) {
    nbl::JsonElement element = {};
    nbl::StringView json = "{\"a\":1";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseErrorEmptyArrayMissingClose) {
    nbl::JsonElement element = {};
    nbl::StringView json = "[1,2";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_TRUE(nbl::parseJson(json, &storage, &element).error());
}

TEST(Json, ParseWhitespaceAroundTokens) {
    nbl::JsonElement element = {};
    nbl::StringView json = "  {  \"key\"  :  42  ,  \"arr\"  :  [  1 , 2  ]  }  ";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 2);
    EXPECT_EQ(element.object.get("key")->number.asUint(), 42);
    EXPECT_EQ(element.object.get("arr")->array.count, 2);
    EXPECT_EQ(element.object.get("arr")->array[0].number.asUint(), 1);
    EXPECT_EQ(element.object.get("arr")->array[1].number.asUint(), 2);
}

TEST(Json, ParseStringWithSpacesAndSpecialChars) {
    nbl::JsonElement element = {};
    nbl::StringView json = "\"  spaces  and !@#$%^&*()  \"";
    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);
    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string, nbl::StringView("  spaces  and !@#$%^&*()  "));
}

TEST(Json, ParseFilter) {

    nbl::JsonElement element = {};
    
    nbl::StringView json = R"({"maxRating":0.0,"minPlayTime":0,"sortBy":"last_played","excludedTags":[],"minRating":0.0,"offset":0,"limit":0,"sortOrder":"descending","minReleaseDate":0,"maxReleaseDate":0,"maxPlayTime":0,"titleFilter":"","tags":[]})";

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 13);

}

TEST(Json, ParseCommand) {

    nbl::JsonElement element = {};
    
    nbl::StringView json = "{\"command\":\"\\\"/mnt/Games/Game/launch.sh\\\"\",\"working_dir\":\"/mnt/Games/Game\"}";

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 2);

}

TEST(Json, ParseNumber) {

    nbl::JsonNumber number = {};

    nbl::StringView json = "7";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    EXPECT_EQ(nbl::parseJsonNumber(stream, &number), nbl::Result::SUCCESS);
    EXPECT_EQ(number.intValue, 7);
}

TEST(Json, ParseUnicode) {

    nbl::JsonElement element = {};

    nbl::StringView json = "\"\\u0027\"";

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(json, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string, "'");
}
