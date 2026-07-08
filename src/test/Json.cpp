#include <gtest/gtest.h>

#include <Core/Result.hpp>
#include <Core/String.hpp>
#include <Platform/Arena.hpp>
#include <Codec/Json.hpp>


TEST(Json, CreateInt) {

    nbl::JsonValue value = nbl::JsonValue::fromInt(42);
    EXPECT_EQ(value.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(value.number.type, nbl::JsonNumberType::SIGNED);
    EXPECT_EQ(value.number.sint, 42);

}

TEST(Json, CreateUint) {

    nbl::JsonValue value = nbl::JsonValue::fromUint(42U);
    EXPECT_EQ(value.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(value.number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(value.number.uint, 42U);

}

TEST(Json, CreateFloat) {

    nbl::JsonValue value = nbl::JsonValue::fromFloat(1.5);
    EXPECT_EQ(value.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(value.number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(value.number.real, 1.5);

}

TEST(Json, CreateString) {
    nbl::JsonValue value = nbl::JsonValue::fromString("Hello, World");
    EXPECT_EQ(value.type, nbl::JsonType::STRING);
    EXPECT_EQ(value.string, "Hello, World");
}

TEST(Json, BuildArray) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);

    nbl::JsonArray arr;
    
    auto builder = nbl::JsonArray::Builder();
    EXPECT_TRUE(builder.addChecked(nbl::JsonValue::fromString("Hello, World")).success());
    EXPECT_TRUE(builder.addChecked(nbl::JsonValue::fromUint(42U)).success());
    EXPECT_TRUE(builder.build(&frame, &arr).success());

    ASSERT_EQ(arr.length(), 2);
    ASSERT_EQ(arr[0]->type, nbl::JsonType::STRING);
    ASSERT_EQ(arr[0]->string, "Hello, World");
    ASSERT_EQ(arr[1]->type, nbl::JsonType::NUMBER);
    ASSERT_EQ(arr[1]->number.asUint(), 42U);

}


TEST(Json, BuildArrayInPlace) {

    nbl::JsonArray arr;
    
    ASSERT_TRUE(nbl::JsonArray::Builder()
        .add(nbl::JsonValue::fromString("Hello, World"))
        .add(nbl::JsonValue::fromUint(42U))
        .build(nullptr, &arr).success());

    ASSERT_EQ(arr.length(), 2);
    ASSERT_EQ(arr[0]->type, nbl::JsonType::STRING);
    ASSERT_EQ(arr[0]->string, "Hello, World");
    ASSERT_EQ(arr[1]->type, nbl::JsonType::NUMBER);
    ASSERT_EQ(arr[1]->number.asUint(), 42U);

}

TEST(Json, BuildObject) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);

    nbl::JsonObject obj;

    ASSERT_TRUE(nbl::JsonObject::Builder()
                .add("name", nbl::JsonValue::fromString("John"))
                .add("age", nbl::JsonValue::fromUint(30U))
                .build(&frame, &obj).success());

    const nbl::JsonValue* nameValue = obj.get("name");
    EXPECT_NE(nameValue, nullptr);
    EXPECT_EQ(nameValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(nameValue->string, "John");

    const nbl::JsonValue* ageValue = obj.get("age");
    EXPECT_NE(ageValue, nullptr);
    EXPECT_EQ(ageValue->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(ageValue->number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(ageValue->number.uint, 30U);

}

TEST(Json, BuildComplex) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);

    nbl::JsonObject obj;

    auto builder = nbl::JsonObject::Builder();
    EXPECT_TRUE(builder.addChecked("name", nbl::JsonValue::fromString("John")).success());
    EXPECT_TRUE(builder.addChecked("age", nbl::JsonValue::fromUint(30U)).success());

    nbl::JsonArray arr;
    EXPECT_TRUE(nbl::JsonArray::Builder()
        .add(nbl::JsonValue::fromString("Hello, World"))
        .add(nbl::JsonValue::fromUint(42U))
        .build(&frame, &arr).success());

    EXPECT_TRUE(builder.addChecked("data", arr).success());
    EXPECT_TRUE(builder.build(&frame, &obj).success());

    const nbl::JsonValue* nameValue = obj.get("name");
    EXPECT_NE(nameValue, nullptr);
    EXPECT_EQ(nameValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(nameValue->string, "John");

    const nbl::JsonValue* ageValue = obj.get("age");
    EXPECT_NE(ageValue, nullptr);
    EXPECT_EQ(ageValue->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(ageValue->number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(ageValue->number.uint, 30U);

    const nbl::JsonValue* dataValue = obj.get("data");
    EXPECT_NE(dataValue, nullptr);
    EXPECT_EQ(dataValue->type, nbl::JsonType::ARRAY);

}

TEST(Json, BuildObjectInPlace) {

    nbl::JsonObject obj;

    auto builder = nbl::JsonObject::Builder();
    EXPECT_TRUE(builder.addChecked("name", nbl::JsonValue::fromString("John")).success());
    EXPECT_TRUE(builder.addChecked("age", nbl::JsonValue::fromUint(30U)).success());
    EXPECT_TRUE(builder.build(nullptr, &obj).success());

    const nbl::JsonValue* nameValue = obj.get("name");
    EXPECT_NE(nameValue, nullptr);
    EXPECT_EQ(nameValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(nameValue->string, "John");

    const nbl::JsonValue* ageValue = obj.get("age");
    EXPECT_NE(ageValue, nullptr);
    EXPECT_EQ(ageValue->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(ageValue->number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(ageValue->number.uint, 30U);

}

TEST(Json, ParseNull) {
    nbl::StringView json = "null";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::NONE);
}

TEST(Json, ParseTrue) {
    nbl::StringView json = "true";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(value.boolean, true);
}

TEST(Json, ParseFalse) {
    nbl::StringView json = "false";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::BOOLEAN);
    EXPECT_EQ(value.boolean, false);
}

TEST(Json, ParseUint) {
    nbl::StringView json = "42";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(value.number.asUint(), 42);
}

TEST(Json, ParseInt) {
    nbl::StringView json = "-42";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(value.number.asInt(), -42);
}

TEST(Json, ParseFloat) {
    nbl::StringView json = "1.5";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::NUMBER);
    EXPECT_EQ(value.number.asFloat(), 1.5);
}

TEST(Json, ParseString) {
    nbl::StringView json = R"("Hello, World")";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::STRING);
    EXPECT_EQ(value.string, nbl::StringView("Hello, World"));
}

TEST(Json, ParseArray) {
    nbl::StringView json = R"([1, 2, 3])";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(value.array.length(), 3);
    EXPECT_EQ(value.array[0]->number.asUint(), 1U);
    EXPECT_EQ(value.array[1]->number.asUint(), 2U);
    EXPECT_EQ(value.array[2]->number.asUint(), 3U);
}


TEST(Json, ParseEmptyArray) {

    nbl::StringView json = R"([])";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(value.array.length(), 0);

}

TEST(Json, ParseArrayInPlace) {
    nbl::StringView json = R"([1, [1, 2, 3], 3])";

    nbl::StringReader reader(json);
    nbl::JsonValue value;
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value, true).success());

    EXPECT_EQ(value.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(value.array.length(), 3);
    EXPECT_EQ(value.array[0]->number.asUint(), 1U);

    EXPECT_EQ(value.array[1]->array.length(), 3U);
    EXPECT_EQ(value.array[1]->array[0]->number.asUint(), 1U);
    EXPECT_EQ(value.array[1]->array[1]->number.asUint(), 2U);
    EXPECT_EQ(value.array[1]->array[2]->number.asUint(), 3U);

    EXPECT_EQ(value.array[2]->number.asUint(), 3U);
}

TEST(Json, ParseLargeArrayInPlace) {
    nbl::StringView json = R"([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40])";

    nbl::StringReader reader(json);
    nbl::JsonValue value;
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value, true).success());

    EXPECT_EQ(value.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(value.array.length(), 40U);
    EXPECT_EQ(value.array[0]->number.asUint(), 1U);

    usize index = 1;
    for(const nbl::JsonValue& element : value.array) {
        EXPECT_EQ(element.number.asUint(), index);
        index++;
    }
}

TEST(Json, ParseArrayInArray) {

    nbl::StringView json = "[[\"test\"]]";
    nbl::StringReader reader(json);

    nbl::JsonValue value;
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value, true).success());

    EXPECT_EQ(value.type, nbl::JsonType::ARRAY);
    EXPECT_EQ(value.array.length(), 1U);
    EXPECT_EQ(value.array[0]->type, nbl::JsonType::ARRAY);
    EXPECT_EQ(value.array[0]->array.length(), 1U);

    EXPECT_EQ(value.array[0]->array[0]->type, nbl::JsonType::STRING);
    EXPECT_EQ(value.array[0]->array[0]->string, "test");
}

TEST(Json, ParseObject) {
    nbl::StringView json = R"({"name": "John", "age": 30})";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024 * 1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_EQ(nbl::parseJson(reader, &frame, &value), nbl::Result::SUCCESS);

    EXPECT_EQ(value.type, nbl::JsonType::OBJECT);

    const nbl::JsonValue* nameValue = value.object.get("name");
    EXPECT_NE(nameValue, nullptr);
    EXPECT_EQ(nameValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(nameValue->string, "John");

    const nbl::JsonValue* ageValue = value.object.get("age");
    EXPECT_NE(ageValue, nullptr);
    EXPECT_EQ(ageValue->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(ageValue->number.asUint(), 30U);
}

TEST(Json, ParseEmptyObject) {
    nbl::StringView json = R"({})";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);
    
    EXPECT_TRUE(nbl::parseJson(reader, &frame, &value).success());

    EXPECT_EQ(value.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(value.object.count(), 0);
}

TEST(Json, ParseObjectInPlace) {
    nbl::StringView json = R"({"name": "John", "subobject": {"key": "value"}, "age": 30})";

    nbl::StringReader reader(json);
    nbl::JsonValue value;
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    
    EXPECT_EQ(nbl::parseJson(reader, &frame, &value, true), nbl::Result::SUCCESS);

    EXPECT_EQ(value.type, nbl::JsonType::OBJECT);

    const nbl::JsonValue* nameValue = value.object.get("name");
    EXPECT_NE(nameValue, nullptr);
    EXPECT_EQ(nameValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(nameValue->string, "John");

    const nbl::JsonValue* subobjectValue = value.object.get("subobject");
    EXPECT_NE(subobjectValue, nullptr);
    EXPECT_EQ(subobjectValue->type, nbl::JsonType::OBJECT);

    const nbl::JsonValue* keyValue = subobjectValue->object.get("key");
    EXPECT_NE(keyValue, nullptr);
    EXPECT_EQ(keyValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(keyValue->string, "value");

    const nbl::JsonValue* ageValue = value.object.get("age");
    EXPECT_NE(ageValue, nullptr);
    EXPECT_EQ(ageValue->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(ageValue->number.asUint(), 30U);
}

TEST(Json, ParseLargeObjectInPlace) {
    nbl::StringView json = R"({"1":1,"2":2,"3":3,"4":4,"5":5,"6":6,"7":7,"8":8,"9":9,"10":10,"11":11,"12":12,"13":13,"14":14,"15":15,"16":16,"17":17,"18":18,"19":19,"20":20,"21":21,"22":22,"23":23,"24":24,"25":25,"26":26,"27":27,"28":28,"29":29,"30":30,"31":31,"32":32,"33":33,"34":34,"35":35,"36":36,"37":37,"38":38,"39":39,"40":40})";

    nbl::StringReader reader(json);
    nbl::JsonValue value;
    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    
    EXPECT_EQ(nbl::parseJson(reader, &frame, &value, true), nbl::Result::SUCCESS);

    EXPECT_EQ(value.type, nbl::JsonType::OBJECT);

    for(usize i = 1; i <= 40; i++) {

        char stringData[3];
        nbl::BufferStream stream(stringData, 3);
        nbl::writeInt(i, stream);

        nbl::StringView key(stream.writerIndex(), stringData);

        const nbl::JsonValue* elementValue = value.object.get(key);
        EXPECT_NE(elementValue, nullptr);
        EXPECT_EQ(elementValue->type, nbl::JsonType::NUMBER);
        EXPECT_EQ(elementValue->number.asUint(), i);
    }

}


TEST(Json, ParseComplex) {
    nbl::StringView json = R"({
        "name": "John",
        "age": 30,
        "isStudent": false,
        "scores": [85.5, 90.0, 78.0],
        "address": {
            "street": "123 Main St",
            "city": "Anytown"
        }
    })";

    nbl::StringReader reader(json);
    nbl::Arena arena;
    nbl::JsonValue value;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::ArenaFrame frame(&arena);

    nbl::Result result = nbl::parseJson(reader, &frame, &value);
    EXPECT_EQ(result, nbl::Result::SUCCESS);
    EXPECT_EQ(value.type, nbl::JsonType::OBJECT);

    const nbl::JsonValue* nameValue = value.object.get("name");
    EXPECT_NE(nameValue, nullptr);
    EXPECT_EQ(nameValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(nameValue->string, "John");

    const nbl::JsonValue* ageValue = value.object.get("age");
    EXPECT_NE(ageValue, nullptr);
    EXPECT_EQ(ageValue->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(ageValue->number.type, nbl::JsonNumberType::UNSIGNED);
    EXPECT_EQ(ageValue->number.uint, 30);

    const nbl::JsonValue* isStudentValue = value.object.get("isStudent");
    EXPECT_NE(isStudentValue, nullptr);
    EXPECT_EQ(isStudentValue->type, nbl::JsonType::BOOLEAN);
    EXPECT_FALSE(isStudentValue->boolean);

    const nbl::JsonValue* scoresValue = value.object.get("scores");
    EXPECT_NE(scoresValue, nullptr);
    EXPECT_EQ(scoresValue->type, nbl::JsonType::ARRAY);
    EXPECT_EQ(scoresValue->array.length(), 3);

    const nbl::JsonValue* score1 = scoresValue->array[0];
    EXPECT_EQ(score1->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(score1->number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(score1->number.real, 85.5);

    const nbl::JsonValue* score2 = scoresValue->array[1];
    EXPECT_EQ(score2->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(score2->number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(score2->number.real, 90.0);

    const nbl::JsonValue* score3 = scoresValue->array[2];
    EXPECT_EQ(score3->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(score3->number.type, nbl::JsonNumberType::FLOAT);
    EXPECT_EQ(score3->number.real, 78.0);

    const nbl::JsonValue* addressValue = value.object.get("address");
    EXPECT_NE(addressValue, nullptr);
    EXPECT_EQ(addressValue->type, nbl::JsonType::OBJECT);

    const nbl::JsonValue* streetValue = addressValue->object.get("street");
    EXPECT_NE(streetValue, nullptr);
    EXPECT_EQ(streetValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(streetValue->string, "123 Main St");

    const nbl::JsonValue* cityValue = addressValue->object.get("city");
    EXPECT_NE(cityValue, nullptr);
    EXPECT_EQ(cityValue->type, nbl::JsonType::STRING);
    EXPECT_EQ(cityValue->string, "Anytown");

}

TEST(Json, WriteNull) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::writeJson(arena, nbl::JsonValue::NULL_VALUE);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));
    EXPECT_EQ(result, "null");
}

TEST(Json, WriteTrue) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::writeJson(arena, nbl::JsonValue::TRUE_VALUE);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));
    EXPECT_EQ(result, "true");
}

TEST(Json, WriteFalse) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::writeJson(arena, nbl::JsonValue::FALSE_VALUE);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));
    EXPECT_EQ(result, "false");
}

TEST(Json, WriteNumber) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::JsonValue value = nbl::JsonValue::fromFloat(1.5);
    nbl::writeJson(arena, value);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));
    EXPECT_EQ(result, "1.5");
   
}

TEST(Json, WriteString) {

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());

    nbl::JsonValue value = nbl::JsonValue::fromString("Hello, World");
    nbl::writeJson(arena, value);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));
    EXPECT_EQ(result, "\"Hello, World\"");
   
}

TEST(Json, WriteArray) {


    nbl::JsonArray arr;
    auto builder = nbl::JsonArray::Builder();

    EXPECT_TRUE(builder.addChecked(nbl::JsonValue::fromString("Hello, World")).success());
    EXPECT_TRUE(builder.addChecked(nbl::JsonValue::fromUint(42U)).success());
    EXPECT_TRUE(builder.build(nullptr, &arr).success());

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::writeJsonArray(arena, arr);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));
    EXPECT_EQ(result, "[\"Hello, World\",42]");
   
}


TEST(Json, WriteObject) {

    nbl::JsonObject obj;
    auto builder = nbl::JsonObject::Builder();
    EXPECT_TRUE(builder.addChecked("key", nbl::JsonValue::fromString("value")).success());
    EXPECT_TRUE(builder.addChecked("num", nbl::JsonValue::fromUint(42U)).success());
    EXPECT_TRUE(builder.build(nullptr, &obj).success());

    nbl::Arena arena;
    EXPECT_TRUE(nbl::Arena::create(1024, &arena).success());
    nbl::writeJsonObject(arena, obj);

    void* begin;
    EXPECT_TRUE(arena.get(0, &begin).success());

    nbl::StringView result(arena.index(), static_cast<char*>(begin));

    nbl::JsonValue parsed;
    nbl::StringReader reader(result);

    nbl::ArenaFrame parseFrame(&arena);
    EXPECT_TRUE(nbl::parseJson(reader, &parseFrame, &parsed).success());

    EXPECT_EQ(parsed.type, nbl::JsonType::OBJECT);
    EXPECT_EQ(parsed.object.count(), 2);

    EXPECT_EQ(parsed.object.get("key")->type, nbl::JsonType::STRING);
    EXPECT_EQ(parsed.object.get("key")->string, "value");
    EXPECT_EQ(parsed.object.get("num")->type, nbl::JsonType::NUMBER);
    EXPECT_EQ(parsed.object.get("num")->number.asUint(), 42U);
   
}
