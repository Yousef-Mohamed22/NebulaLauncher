#include <gtest/gtest.h>
#include <Codec/Json.hpp>

TEST(JsonBuilder, EmptyObject) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;
    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 0);
}
TEST(JsonBuilder, EmptyArray) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonArray::Builder builder{};
    EXPECT_EQ(nbl::JsonArray::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonArray obj;
    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.count, 0);
}

TEST(JsonBuilder, ObjectWithPrimitives) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;

    EXPECT_EQ(builder.add("int", 42), nbl::Result::SUCCESS);
    EXPECT_EQ(builder.add("bool", true), nbl::Result::SUCCESS);
    EXPECT_EQ(builder.add("float", 12.34), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 3);
    EXPECT_EQ(obj.getNumber("int").asInt(), 42);
    EXPECT_EQ(obj.getBoolean("bool"), true);
    EXPECT_EQ(obj.getNumber("float").asFloat(), 12.34);
}

TEST(JsonBuilder, ArrayWithPrimitives) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonArray::Builder builder{};
    EXPECT_EQ(nbl::JsonArray::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.add(42), nbl::Result::SUCCESS);
    EXPECT_EQ(builder.add(true), nbl::Result::SUCCESS);
    EXPECT_EQ(builder.add(12.34), nbl::Result::SUCCESS);

    nbl::JsonArray arr;
    EXPECT_EQ(builder.build(&arr), nbl::Result::SUCCESS);

    EXPECT_EQ(arr.count, 3);
    EXPECT_EQ(arr.getNumber(0).asInt(), 42);
    EXPECT_EQ(arr.getBoolean(1), true);
    EXPECT_EQ(arr.getNumber(2).asFloat(), 12.34);
}

TEST(JsonBuilder, ArrayWithStrings) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonArray::Builder builder{};
    EXPECT_EQ(nbl::JsonArray::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.add("Hello"), nbl::Result::SUCCESS);
    EXPECT_EQ(builder.add("World"), nbl::Result::SUCCESS);

    nbl::JsonArray arr;
    EXPECT_EQ(builder.build(&arr), nbl::Result::SUCCESS);

    EXPECT_EQ(arr.count, 2);
    EXPECT_EQ(arr.getString(0), "Hello");
    EXPECT_EQ(arr.getString(1), "World");
}



TEST(JsonBuilder, ObjectWithStrings) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;

    EXPECT_EQ(builder.add("str", "Hello"), nbl::Result::SUCCESS);
    EXPECT_EQ(builder.add("str2", "World"), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 2);
    EXPECT_EQ(obj.getString("str"), "Hello");
    EXPECT_EQ(obj.getString("str2"), "World");
}

TEST(JsonBuilder, ObjectWithArray) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;

    EXPECT_EQ(builder.addArray("arr", [&](auto& arr) {
        return arr.add(1);
    }), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 1);
    EXPECT_TRUE(obj.has("arr"));
    EXPECT_EQ(obj.getArray("arr").count, 1);
    EXPECT_EQ(obj.getArray("arr").getNumber(0).asInt(), 1);
}

TEST(JsonBuilder, ArrayWithSubarray) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonArray::Builder builder{};
    EXPECT_EQ(nbl::JsonArray::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonArray arr;

    EXPECT_EQ(builder.addArray([&](auto& arr) {
        return arr.add(1);
    }), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&arr), nbl::Result::SUCCESS);
    EXPECT_EQ(arr.count, 1);
    EXPECT_EQ(arr.getArray(0).count, 1);
    EXPECT_EQ(arr.getArray(0).getNumber(0).asInt(), 1);
}

TEST(JsonBuilder, ObjectWithSubobject) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;

    EXPECT_EQ(builder.addObject("obj", [&](auto& obj) {
        return obj.add("key", 1);
    }), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 1);
    EXPECT_TRUE(obj.has("obj"));
    EXPECT_EQ(obj.getObject("obj").entryCount, 1);
    EXPECT_EQ(obj.getObject("obj").getNumber("key").asInt(), 1);
}

TEST(JsonBuilder, ArrayWithObject) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonArray::Builder builder{};
    EXPECT_EQ(nbl::JsonArray::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonArray arr{};
    EXPECT_EQ(builder.addObject([&](auto& obj) {
        return obj.add("key", 1);
    }), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&arr), nbl::Result::SUCCESS);
    EXPECT_EQ(arr.count, 1);
    EXPECT_EQ(arr.getObject(0).entryCount, 1);
    EXPECT_EQ(arr.getObject(0).getNumber("key").asInt(), 1);
}

TEST(JsonBuilder, ObjectWithNestedSubobject) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;

    EXPECT_EQ(builder.addObject("obj", [&](auto& obj) {
        return obj.addObject("key", [&](auto& obj2) {
            return obj2.add("test", 1);
        });
    }), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 1);
    EXPECT_TRUE(obj.has("obj"));
    EXPECT_EQ(obj.getObject("obj").entryCount, 1);
    EXPECT_EQ(obj.getObject("obj").getObject("key").getNumber("test").asInt(), 1);
}

TEST(JsonBuilder, ObjectWithNestedArrayInSubobject) {

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::JsonObject::Builder builder{};
    EXPECT_EQ(nbl::JsonObject::Builder::create(&frame, &builder), nbl::Result::SUCCESS);

    nbl::JsonObject obj;

    EXPECT_EQ(builder.addObject("obj", [&](auto& obj) {
        return obj.addArray("key", [&](auto& arr) {
            return arr.add(1);
        });
    }), nbl::Result::SUCCESS);

    EXPECT_EQ(builder.build(&obj), nbl::Result::SUCCESS);
    EXPECT_EQ(obj.entryCount, 1);
    EXPECT_TRUE(obj.has("obj"));
    EXPECT_EQ(obj.getObject("obj").entryCount, 1);
    EXPECT_EQ(obj.getObject("obj").getArray("key").getNumber(0).asInt(), 1);
}
