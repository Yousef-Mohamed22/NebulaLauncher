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


#include <Codec/Json.hpp>
#include <gtest/gtest.h>

TEST(Json, ParseNull) {

    nbl::JsonElement element = {};

    nbl::StringView json = "null";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::NONE);
}

TEST(Json, ParseTrue) {

    nbl::JsonElement element = {};

    nbl::StringView json = "true";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(element.boolean, true);
}

TEST(Json, ParseFalse) {

    nbl::JsonElement element = {};

    nbl::StringView json = "false";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(element.boolean, false);
}

TEST(Json, ParseInt) {

    nbl::JsonElement element = {};

    nbl::StringView json = "-145";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::SIGNED);
    EXPECT_EQ(element.number.intValue, -145);
}

TEST(Json, ParseUint) {

    nbl::JsonElement element = {};

    nbl::StringView json = "145";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_NE(element.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(element.number.uintValue, 145);
}

TEST(Json, ParseFloat) {

    nbl::JsonElement element = {};

    nbl::StringView json = "1.45";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(element.number.floatValue, 1.45);
}

TEST(Json, ParseString) {

    nbl::JsonElement element = {};

    nbl::StringView json = "\"Hello, World\"";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::STRING);
    EXPECT_EQ(element.string, nbl::StringView("Hello, World"));
}

TEST(Json, ParseArray) {

    nbl::JsonElement element = {};
    
    nbl::StringView json = "[1,2,3]";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(element.array.count, 3);
    EXPECT_EQ(element.array[0].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[0].number.asUint(), 1);
    EXPECT_EQ(element.array[1].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[1].number.asUint(), 2);
    EXPECT_EQ(element.array[2].type, nbl::JsonType::NUMBER);
    EXPECT_EQ(element.array[2].number.asUint(), 3);
}


TEST(Json, ParseObject) {

    nbl::JsonElement element = {};
    
    nbl::StringView json = "{\"key\":99}";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

    EXPECT_EQ(element.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(element.object.entryCount, 1);

    const nbl::JsonElement* e = element.object.get("key");
    EXPECT_NE(e, nullptr);
    EXPECT_EQ(e->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(e->number.asUint(), 99);
}


TEST(Json, ParseSubObject) {

    nbl::JsonElement element = {};
    
    nbl::StringView json = "{\"sub\":{\"key\":\"value\"}}";
    nbl::BufferStream stream(const_cast<char*>(json.data), json.length);

    nbl::Arena storage;
    EXPECT_EQ(nbl::Arena::create(1024, &storage), nbl::Result::SUCCESS);
    EXPECT_EQ(nbl::parseJson(stream, &storage, &element), nbl::Result::SUCCESS);

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

TEST(Json, ParseFilter) {

    nbl::JsonElement element = {};
    
    nbl::StringView json = "{\"maxRating\":0.0,\"minPlayTime\":0,\"sortBy\":\"last_played\",\"excludedTags\":[],\"minRating\":0.0,\"offset\":0,\"limit\":0,\"sortOrder\":\"descending\",\"minReleaseDate\":0,\"maxReleaseDate\":0,\"maxPlayTime\":0,\"titleFilter\":\"\",\"tags\":[]}";

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
