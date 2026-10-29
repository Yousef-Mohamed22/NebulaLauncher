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

#include "RuntimeManager.hpp"
#include "Arguments.hpp"

#include <Platform/Time.hpp>
#include <Platform/File.hpp>

namespace {


constexpr nbl::StringView RUNTIME_MANIFEST_URL = "https://launchermeta.mojang.com/v1/products/java-runtime/2ec0cc96c44e5a76b9c8b7c39df7210883d12871/all.json";


nbl::StringView _archStr() {

    static char archBuffer[16]{};
    usize index = 0;
    nbl::mc::OsInfo osInfo = nbl::mc::OsInfo::current();

    if(osInfo.name == "macos") {
        memcpy(archBuffer, "mac-os", 6);
        index = 6;
    } else {
        memcpy(archBuffer, osInfo.name.data(), osInfo.name.length());
        index = osInfo.name.length();
    }

    switch(osInfo.arch) {
        case nbl::mc::Architecture::AMD64: {
            if(osInfo.name == "windows") {
                memcpy(archBuffer + index, "-x64", 4);
                index += 4;
            }
            break;
        } 
        case nbl::mc::Architecture::X86: {
            if(osInfo.name == "linux") {
                memcpy(archBuffer + index, "-i386", 5);
                index += 5;
            } else if(osInfo.name == "windows") {
                memcpy(archBuffer + index, "-x86", 4);
                index += 4;
            }
            break;
        }
        case nbl::mc::Architecture::AARCH64: {
            memcpy(archBuffer + index, "-arm64", 6);
            index += 6;
            break;
        }
    }

    return nbl::StringView(archBuffer, index);
}


}


nbl::Result nbl::mc::downloadRuntime(DownloadManager* client, StringView runtimeFolder, StringView runtimeComponent) {

    StringView arch = _archStr();

    AllocationFrame frame(Arena::threadLocal());
    StringView manifestPath;
    NBL_CHECK(StringBuilder(&frame).append(runtimeFolder).append("/manifest.json").buildNullTerminated(&manifestPath));

    client->downloadNow(manifestPath, RUNTIME_MANIFEST_URL);
    
    FileChannel manifestFile{};
    NBL_CHECK(FileChannel::open(manifestPath, &manifestFile, FileOpenFlag::READ));

    JsonElement root{};
    NBL_CHECK(parseJson(manifestFile.reader(), &frame, &root));

    if(root.type() != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    JsonArray components = root.asObject().getObject(arch).getArray(runtimeComponent);
    if(components.count == 0) return nbl::Result::NOT_FOUND;

    StringView url = components.getObject(0).getObject("manifest").getString("url");
    if(url.length() == 0) return nbl::Result::NOT_FOUND;


    StringView componentManifestPath;
    NBL_CHECK(StringBuilder(&frame).append(runtimeFolder).append('/').append(runtimeComponent).append('/').append(arch).append(".json").buildNullTerminated(&componentManifestPath));

    client->downloadNow(componentManifestPath, url);

    FileChannel componentManifestFile{};
    NBL_CHECK(FileChannel::open(componentManifestPath, &componentManifestFile, FileOpenFlag::READ));

    JsonElement componentRoot{};
    NBL_CHECK(parseJson(componentManifestFile.reader(), &frame, &componentRoot));

    if(componentRoot.type() != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

    for(JsonObject::Entry& ent : componentRoot.asObject().getObject("files")) {
        if(ent.value.type() != JsonType::OBJECT) continue;
        StringView type = ent.value.asObject().getString("type");
        if(type == "directory") continue;


        AllocationFrame entryFrame(Arena::threadLocal());
        StringView filePath;
        StringBuilder(&entryFrame).append(runtimeFolder).append('/').append(runtimeComponent).append('/').append(arch).append('/').append(ent.key).buildNullTerminated(&filePath);

        if(type == "link") {

            usize last = 0;
            NBL_CHECK(filePath.findLast('/', &last));
            NBL_CHECK(makeDirectories(filePath.slice(0, last)));
            
            nbl::createLink(ent.value.asObject().getString("target"), filePath);

        } else if(type == "file") {

            StringView sha1 = ent.value.asObject().getObject("downloads").getObject("raw").getString("sha1");
            StringView url = ent.value.asObject().getObject("downloads").getObject("raw").getString("url");

            bool executable = ent.value.asObject().getBoolean("executable");
            NBL_CHECK(client->verifyOrDownload(sha1, filePath, url, executable));

        } else { 

            nbl::log(nbl::LogLevel::WARN, "Unknown file type: ", type);

        }
    }

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::mc::getRuntimePath(StringView runtimeFolder, StringView runtimeComponent, StringBuilder* out) {

    out->append(runtimeFolder).append('/').append(runtimeComponent).append('/').append(_archStr()).append("/bin/");
    
    OsInfo info = OsInfo::current();
    if(info.name == "windows") {
        return out->appendChecked("javaw.exe");
    } else {
        return out->appendChecked("java");
    }
}
