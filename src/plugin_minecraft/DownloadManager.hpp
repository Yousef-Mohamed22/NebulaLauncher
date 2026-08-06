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
