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

#include "AssetManager.hpp"
#include "DownloadManager.hpp"
#include "Util.hpp"

#include <Platform/File.hpp>
#include <Utility/StringBuilder.hpp>
#include <Utility/Hash.hpp>
#include <Codec/Json.hpp>


nbl::Result nbl::mc::downloadAssets(nbl::mc::DownloadManager* client, StringView assetFolder, StringView id, StringView sha1, StringView url) {

    AllocationFrame frame(Arena::threadLocal());

    StringView indexFile;
    StringBuilder(&frame).append(assetFolder).append("/indexes/").append(id).append(".json").buildNullTerminated(&indexFile);

    client->verifyOrDownloadNow(sha1, indexFile, url);

    FileChannel channel;
    NBL_CHECK(FileChannel::open(indexFile, &channel, FileOpenFlag::READ));

    JsonElement indexJson;
    NBL_CHECK(parseJson(channel.reader(), &frame, &indexJson));
    if(indexJson.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;
    
    JsonObject objects = indexJson.object.getObject("objects");
    for(JsonObject::Entry ent : objects) {
        if(ent.value.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

        StringView hash = ent.value.object.getString("hash");
        u64 size = ent.value.object.getNumber("size").uintValue;

        if(hash.length != 40 || size == 0) {
            return nbl::Result::FORMAT_ERROR;
        }


        StringView filePath;
        NBL_CHECK(StringBuilder(&frame).append(assetFolder).append("/objects/").append(hash.slice(0, 2)).append("/").append(hash).buildNullTerminated(&filePath));

        StringView folderPath = filePath.slice(0, filePath.length - hash.length - 1);
        nbl::makeDirectories(folderPath);

        StringView fileUrl;
        NBL_CHECK(StringBuilder(&frame).append("https://resources.download.minecraft.net/").append(hash.slice(0, 2)).append("/").append(hash).buildNullTerminated(&fileUrl));
        
        NBL_CHECK(client->verifyOrDownload(hash, filePath, fileUrl));
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::mc::verifyStrict(StringView assetFolder, StringView id, StringView sha1) {

    AllocationFrame frame(Arena::threadLocal());

    StringView indexFile;
    StringBuilder(&frame).append(assetFolder).append("/indexes/").append(id).append(".json").buildNullTerminated(&indexFile);

    FileChannel channel;
    NBL_CHECK(FileChannel::open(indexFile, &channel, FileOpenFlag::READ));

    JsonElement indexJson;
    NBL_CHECK(parseJson(channel.reader(), &frame, &indexJson));
    if(indexJson.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    JsonObject objects = indexJson.object.getObject("objects");
    for(JsonObject::Entry ent : objects) {
        if(ent.value.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

        StringView hash = ent.value.object.getString("hash");
        u64 size = ent.value.object.getNumber("size").uintValue;

        if(hash.length != 40 || size == 0) {
            return nbl::Result::FORMAT_ERROR;
        }

        StringView filePath;
        NBL_CHECK(StringBuilder(&frame).append(assetFolder).append("/objects/").append(hash.slice(0, 2)).append("/").append(hash).buildNullTerminated(&filePath));

        Array<u8, 20> parsedFileSha1;
        nbl::mc::parseHex(hash, parsedFileSha1);

        if(nbl::fileExists(filePath)) {

            Array<u8, 20> calculatedSha1;
            nbl::fileSha1(filePath, calculatedSha1);

            if(memcmp(parsedFileSha1.data, calculatedSha1.data, 20) != 0) {
                nbl::log(nbl::LogLevel::ERROR, "Found SHA1 mismatch for ", filePath, "!");
                return nbl::Result::IO_ERROR;
            }
        } else {
            nbl::log(nbl::LogLevel::ERROR, "Found missing file: ", filePath);
            return nbl::Result::IO_ERROR;
        }
    }
    return nbl::Result::SUCCESS;
}

