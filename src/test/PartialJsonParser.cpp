#include <gtest/gtest.h>

#include <Codec/Json.hpp>


TEST(PartialJsonParser, ParseNumber) {

    nbl::StringView json = "42";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::JsonNumber value;

    EXPECT_TRUE(parser.readNumber(&value).result().success());
    EXPECT_EQ(value.asUint(), 42U);
}

TEST(PartialJsonParser, ParseString) {

    nbl::StringView json = R"("Hello, World")";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder builder(&frame);

    EXPECT_TRUE(parser.readString(&builder).result().success());

    nbl::StringView result;
    EXPECT_TRUE(builder.buildNullTerminated(&result).success());

    EXPECT_EQ(result, "Hello, World");
}

TEST(PartialJsonParser, ParseTrue) {

    nbl::StringView json = "true";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    bool value;
    EXPECT_TRUE(parser.readBoolean(&value).result().success());
    EXPECT_TRUE(value);
}

TEST(PartialJsonParser, ParseFalse) {

    nbl::StringView json = "false";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    bool value;
    EXPECT_TRUE(parser.readBoolean(&value).result().success());
    EXPECT_FALSE(value);
}

TEST(PartialJsonParser, ParseArray) {

    nbl::StringView json = R"([1, 2, 3])";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    u64 v1;
    u32 v2;
    u16 v3;

    EXPECT_TRUE(parser.beginArray()
        .readUint(&v1)
        .readUint(&v2)
        .readUint(&v3)
        .endArray()
        .result().success());

    EXPECT_EQ(v1, 1U);
    EXPECT_EQ(v2, 2U);
    EXPECT_EQ(v3, 3U);
}

TEST(PartialJsonParser, ParseObject) {

    nbl::StringView json = R"({"key1": "value1", "key2": 42})";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    nbl::StringBuilder keyBuilder(&frame);
    EXPECT_TRUE(parser.beginObject().readKey(&keyBuilder).result().success());
    nbl::StringView key1;
    EXPECT_TRUE(keyBuilder.build(&key1).success());
    EXPECT_EQ(key1, "key1");

    nbl::StringBuilder valueBuilder(&frame);
    EXPECT_TRUE(parser.readString(&valueBuilder).result().success());
    nbl::StringView value1;
    EXPECT_TRUE(valueBuilder.buildNullTerminated(&value1).success());
    EXPECT_EQ(value1, "value1");

    nbl::StringBuilder keyBuilder2(&frame);
    EXPECT_TRUE(parser.readKey(&keyBuilder2).result().success());
    nbl::StringView key2;
    EXPECT_TRUE(keyBuilder2.build(&key2).success());
    EXPECT_EQ(key2, "key2");

    u64 numberValue;
    EXPECT_TRUE(parser.readUint(&numberValue).result().success());
    EXPECT_EQ(numberValue, 42U);

    EXPECT_TRUE(parser.endObject().result().success());
}

TEST(PartialJsonParser, SkipKey) {

    nbl::StringView json = R"({"key1": "value1", "key2": 42})";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());
    nbl::StringBuilder valueBuilder(&frame);

    EXPECT_TRUE(parser.beginObject().skipKey().readString(&valueBuilder).result().success());
    nbl::StringView value1;
    EXPECT_TRUE(valueBuilder.buildNullTerminated(&value1).success());
    EXPECT_EQ(value1, "value1");

    u64 numberValue;
    EXPECT_TRUE(parser.skipKey().readUint(&numberValue).result().success());
    EXPECT_EQ(numberValue, 42U);

    EXPECT_TRUE(parser.endObject().result().success());
}

TEST(PartialJsonParser, SkipValue) {

    nbl::StringView json = R"({"key1": "value1", "key2": 42})";

    nbl::StringReader reader(json);
    nbl::PartialJsonParser parser = nbl::PartialJsonParser(reader);

    EXPECT_TRUE(parser.beginObject().skipKey().skipValue().skipKey().skipValue().endObject().result().success());
}
