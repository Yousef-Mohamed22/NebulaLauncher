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

#include "SchemaBin.hpp"

#include <Codec/Json.hpp>
#include <Core/Result.hpp>
#include <Core/Stream.hpp>
#include <Platform/Allocator.hpp>
#include <Platform/File.hpp>
#include <Utility/StringBuilder.hpp>

namespace {


enum _EntryType {
    NONE = 0,
    STRING = 1,
    INT32 = 2,
    FLOAT32 = 3,
    POINTER = 4,
    WIDE_STRING = 5,
    COLOR = 6,
    UINT64 = 7,
    END = 8
};



nbl::Result _parseSchemaObject(nbl::Reader reader, nbl::JsonObject::Builder& builder) {

    u8 stringBuffer[4096]{};
    nbl::InPlaceAllocator alloc(stringBuffer, 4096);

    while(true) {

        u8 type;
        NBL_CHECK(reader.readAll(&type, sizeof(u8)));

        if(type == END) {
            return nbl::Result::SUCCESS;
        }

        alloc.popTo(0);
        nbl::StringBuilder keyBuilder(&alloc);

        char c;
        while(true) {
            NBL_CHECK(reader.readAll(&c, sizeof(char)));
            if(c == '\0') break;
            keyBuilder.append(c);
        }

        nbl::StringView key{};
        NBL_CHECK(keyBuilder.buildNullTerminated(&key));

        switch(type) {
            case NONE: 
                NBL_CHECK(builder.addObject(key, [&](auto& obj) -> nbl::Result {
                    return _parseSchemaObject(reader, obj);
                }, 1));
                break;
            case STRING: {
                nbl::StringBuilder valueBuilder(&alloc);
                char c;
                while(true) {
                    NBL_CHECK(reader.readAll(&c, sizeof(char)));
                    if(c == '\0') break;
                    valueBuilder.append(c);
                }

                nbl::StringView value{};
                NBL_CHECK(valueBuilder.buildNullTerminated(&value));
                NBL_CHECK(builder.add(key, value));
                break;
            }
            case INT32: {
                i32 value;
                NBL_CHECK(reader.readAll(&value, sizeof(i32)));
                NBL_CHECK(builder.add(key, value));
                break;
            }
            case FLOAT32: {
                f32 value;
                NBL_CHECK(reader.readAll(&value, sizeof(f32)));
                NBL_CHECK(builder.add(key, value));
                break;
            }
            case COLOR:
            case POINTER: {
                u32 value;
                NBL_CHECK(reader.readAll(&value, sizeof(u32)));
                NBL_CHECK(builder.add(key, value));
                break;
            }
            case WIDE_STRING:
                return nbl::Result::UNIMPLEMENTED;
            case UINT64: {
                u32 value;
                NBL_CHECK(reader.readAll(&value, sizeof(u64)));
                NBL_CHECK(builder.add(key, value));
                break;
            }
            case END:
                return nbl::Result::FORMAT_ERROR;
        }
    }
}


}


nbl::Result parseSchemaBin(nbl::StringView path, nbl::JsonObject* out, nbl::LinearAllocator* allocator) {


    nbl::FileChannel channel{};
    NBL_CHECK(nbl::FileChannel::open(path, &channel, nbl::FileOpenFlag::READ));

    u8 buffer[4096];
    nbl::BufferedReader reader(channel.reader(), buffer, 4096);

    nbl::JsonObject::Builder builder{};
    NBL_CHECK(nbl::JsonObject::Builder::create(allocator, &builder));
    
    NBL_CHECK(_parseSchemaObject(reader, builder));

    return builder.build(out, 1);
}
