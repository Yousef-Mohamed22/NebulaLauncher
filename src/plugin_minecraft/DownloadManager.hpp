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

#pragma once

#include <Network/HttpClient.hpp>
#include <Plugin/ProcessTracker.hpp>

namespace nbl::mc {


class DownloadManager {
public:

    ~DownloadManager() { awaitAll(); }

    Result verifyOrDownload(StringView sha1, StringView path, StringView url, bool executable = false);
    Result verifyOrDownloadNow(StringView sha1, StringView path, StringView url);
    Result downloadNow(StringView path, StringView url);
    Result download(StringView path, StringView url);
    Result awaitAll();

    static Result create(u32 maxRequests, u32 maxConnections, u32 maxRetries, u32 estimatedTasks, ProcessTracker* tracker, DownloadManager* out);

    inline void refineEstimate(i32 offset) {
        _mEstimatedTasks += offset;
    }

    inline void dropEstimate() {
        _mEstimatedTasks = 0;
    }

private:

    static void _callback(void* userData, Result res, i32 code);

    HttpClient _mClient;
    ProcessTracker* _pTracker;
    u32 _mTasks;
    u32 _mEstimatedTasks;
    u32 _mComplete;

};


}
