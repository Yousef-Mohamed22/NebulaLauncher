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

#include "LoggingManager.hpp"

#include <Platform/File.hpp>
#include <Utility/Placeholder.hpp>
#include <Utility/Hash.hpp>

nbl::Result nbl::mc::prepareLogging(StringView folder, JsonObject loggingInfo, DownloadManager* client, StringBuilder* outArg) {

    StringView argument = loggingInfo.getString("argument");
    JsonObject file = loggingInfo.getObject("file");
    StringView id = file.getString("id");

    StringView path{};
    char pathBuf[4096];
    InPlaceAllocator alloc(pathBuf, 4096);
    NBL_CHECK(StringBuilder(&alloc).append(folder).append('/').append(id).buildNullTerminated(&path));
    NBL_CHECK(client->verifyOrDownload(file.getString("sha1"), path, file.getString("url")));
  
    return nbl::formatPlaceholders(argument, outArg, nbl::Array{
        nbl::Placeholder{.key="path", .value=path}
    });
}
