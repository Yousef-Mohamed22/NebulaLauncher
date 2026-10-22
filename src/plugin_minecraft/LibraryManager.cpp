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

#include "LibraryManager.hpp"
#include "Arguments.hpp"
#include "Util.hpp"

#include <Codec/Json.hpp>
#include <Utility/Hash.hpp>
#include <Platform/File.hpp>
#include <Platform/PlatformDefines.hpp>
#include <Core/Span.hpp>
#include <Core/Array.hpp>



nbl::Result nbl::mc::downloadLibraries(DownloadManager* client, StringView libraryFolder, const VersionRef& ref, FeatureSet features, AllocationFrame* classpath) {

    usize i = 0;
    return ref.enumerateLibraries([&](JsonElement ele) -> Result {
        if(ele.type != JsonType::OBJECT) {
            return nbl::Result::FORMAT_ERROR;
        }

        StringView name = ele.object.getString("name");
        if(ele.object.has("rules")) {
            bool canUse = true;
            for(const JsonElement& ruleEle : ele.object.getArray("rules")) {
                if(ruleEle.type != JsonType::OBJECT) continue;

                Rule rule;
                NBL_CHECK(Rule::parse(ruleEle.object, &rule));

                if(!rule.canUse(OsInfo::current(), features)) {
                    canUse = false; 
                    break;
                }
            }
            if(!canUse) return nbl::Result::SUCCESS;
        }

        StringView path;
        StringView sha1;
        StringView url;

        AllocationFrame frame(Arena::threadLocal());

        JsonObject artifact = ele.object.getObject("downloads").getObject("artifact");
        if(artifact.entryCount > 0) {
            path = artifact.getString("path");
            sha1 = artifact.getString("sha1");
            url = artifact.getString("url");

        } else { // Legacy format

            StringView baseUrl = ele.object.getString("url");
            sha1 = ele.object.getString("sha1");

            // TODO: Parse maven artifact coordinates, convert to maven url.

            usize firstColonIndex;
            NBL_CHECK(name.findFirst(':', &firstColonIndex, 0));

            usize lastColonIndex;
            NBL_CHECK(name.findFirst(':', &lastColonIndex, firstColonIndex + 1));

            StringView group = name.slice(0, firstColonIndex);
            StringView artifact = name.slice(firstColonIndex + 1, lastColonIndex - firstColonIndex - 1);
            StringView version = name.tail(lastColonIndex + 1);

            StringView groupUrl;
            NBL_CHECK(StringBuilder(&frame).append(group).replace('.',"/").build(&groupUrl));

            NBL_CHECK(StringBuilder(&frame)
                    .append(groupUrl)
                    .append('/')
                    .append(artifact)
                    .append('/')
                    .append(version)
                    .append('/')
                    .append(artifact)
                    .append('-')
                    .append(version)
                    .append(".jar")
                    .buildNullTerminated(&path));

            NBL_CHECK(StringBuilder(&frame)
                    .append(baseUrl)
                    .append(path)
                    .buildNullTerminated(&url));
 
        }

        // TODO: Classifiers/Natives, Extraction/Exclude (for old versions)

        if(i > 0) {
            NBL_CHECK(classpath->writer().writeChar(CLASSPATH_SEPARATOR));
        }
        
        if(url.empty() || path.empty()) {
            return nbl::Result::FORMAT_ERROR;
        }

        StringView filePath;
        nbl::StringBuilder(classpath).append(libraryFolder).append('/').append(path).buildNullTerminated(&filePath);

        if(sha1.empty()) {
            NBL_CHECK(client->download(filePath, url));
        } else {
            NBL_CHECK(client->verifyOrDownload(sha1, filePath, url));
        }

        classpath->pop(1); // Remove the null-terminator
        i++;

        return nbl::Result::SUCCESS;
    });


}

nbl::Result nbl::mc::verifyStrict(StringView libraryFolder, const VersionRef& ref, FeatureSet features) {

    usize i = 0;
    return ref.enumerateLibraries([&](JsonElement ele) -> Result {
        if(ele.type != JsonType::OBJECT) return nbl::Result::FORMAT_ERROR;

        StringView name = ele.object.getString("name");
        if(ele.object.has("rules")) {
            bool canUse = true;
            for(const JsonElement& ruleEle : ele.object.getArray("rules")) {
                if(ruleEle.type != JsonType::OBJECT) continue;

                Rule rule;
                NBL_CHECK(Rule::parse(ruleEle.object, &rule));

                if(!rule.canUse(OsInfo::current(), features)) {
                    canUse = false; 
                    break;
                }
            }
            if(!canUse) return nbl::Result::SUCCESS;
        }

        JsonObject artifact = ele.object.getObject("downloads").getObject("artifact");
        StringView path = artifact.getString("path");
        StringView sha1 = artifact.getString("sha1");

        nbl::AllocationFrame frame(nbl::Arena::threadLocal());

        StringView filePath;
        nbl::StringBuilder(&frame).append(libraryFolder).append('/').append(path).buildNullTerminated(&filePath);

        if(nbl::fileExists(filePath)) {

            Array<u8, 20> parsedSha1;
            NBL_CHECK(parseHex(sha1, parsedSha1));

            Array<u8, 20> calculatedSha1;
            NBL_CHECK(fileSha1(filePath, calculatedSha1));

            if(memcmp(parsedSha1.data, calculatedSha1.data, 20) != 0) {
                return nbl::Result::IO_ERROR;
            }

        } else {
            return nbl::Result::IO_ERROR;
        }
        i++;
        return nbl::Result::SUCCESS;
    });
}
