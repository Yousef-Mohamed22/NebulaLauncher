#include "DownloadManager.hpp"
#include "Platform/File.hpp"
#include "Util.hpp"

#include <Core/String.hpp>
#include <Utility/Hash.hpp>
#include <Platform/Logging.hpp>
#include <Platform/Mutex.hpp>

nbl::Result nbl::mc::DownloadManager::verifyOrDownload(StringView sha1, StringView path, StringView url, bool executable) {

    _mTasks++;

    if(nbl::fileExists(path)) {
        nbl::Array<u8, 20> parsedSha1{};
        NBL_CHECK(parseHex(sha1, parsedSha1));
        
        nbl::Array<u8, 20> calculatedSha1{};
        nbl::Result res = fileSha1(path, calculatedSha1);

        if(res.error() || memcmp(parsedSha1.data, calculatedSha1.data, 20)) {
            return _mClient.downloadFile(url, path, {
                .doneCallback = _callback, 
                .userData = this,
                .markExecutable = executable
            });
        }
    } else {
        return _mClient.downloadFile(url, path, {
            .doneCallback = _callback, 
            .userData = this,
            .markExecutable = executable
        });
    }

    _mComplete++;
    _pTracker->pushProgress(static_cast<f64>(_mComplete) / static_cast<f64>(nbl::max(_mTasks, _mEstimatedTasks)));

    return nbl::Result::SUCCESS;
}


nbl::Result nbl::mc::DownloadManager::verifyOrDownloadNow(StringView sha1, StringView path, StringView url) {

    nbl::Array<u8, 20> parsedSha1{};
    NBL_CHECK(parseHex(sha1, parsedSha1));

    if(nbl::fileExists(path)) {
        
        nbl::Array<u8, 20> calculatedSha1{};
        NBL_CHECK(fileSha1(path, calculatedSha1));

        if(memcmp(parsedSha1.data, calculatedSha1.data, 20)) {
            NBL_CHECK(downloadNow(path, url));
        } 
    } else {
        NBL_CHECK(downloadNow(path, url));
    }

    nbl::Array<u8, 20> calculatedSha1{};
    NBL_CHECK(fileSha1(path, calculatedSha1));

    if(memcmp(parsedSha1.data, calculatedSha1.data, 20)) {
        return nbl::Result::IO_ERROR;
    }

    return nbl::Result::SUCCESS;
}
nbl::Result nbl::mc::DownloadManager::downloadNow(StringView path, StringView url) {

    u64 modifyTime = nbl::modifyTime(path);

    Latch latch;
    NBL_CHECK(Latch::create(1, &latch));

    NBL_CHECK(_mClient.downloadFile(url, path, {
        .doneCallback = [](void* ud, Result res, i32 code) {
            static_cast<Latch*>(ud)->decrement();
        },
        .userData = &latch,
        .ifModifiedSince = modifyTime
    }));

    latch.wait();
    return nbl::Result::SUCCESS;
}
nbl::Result nbl::mc::DownloadManager::download(StringView path, StringView url) {

    u64 modifyTime = nbl::modifyTime(path);

    _mTasks++;
    NBL_CHECK(_mClient.downloadFile(url, path, {
        .doneCallback = _callback,
        .userData = this,
        .ifModifiedSince = modifyTime
    }));

    return nbl::Result::SUCCESS;
}

nbl::Result nbl::mc::DownloadManager::awaitAll() {
    dropEstimate();
    u32 failures;
    _mClient.awaitAll(&failures);
    if(failures > 0) return nbl::Result::IO_ERROR;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::mc::DownloadManager::create(u32 maxRequests, u32 maxConnections, u32 maxRetries, u32 estimatedTasks, ProcessTracker *tracker, DownloadManager* out) {

    u64 maxFiles = 0;
    NBL_CHECK(nbl::getOpenFileLimit(&maxFiles, nullptr));

    NBL_CHECK(nbl::HttpClient::create(maxRequests, maxConnections, maxRetries, nbl::min(maxRequests, static_cast<u32>(maxFiles - 16)), &out->_mClient, HttpClientFlag::CREATE_FILE_PARENTS));
    out->_pTracker = tracker;
    out->_mTasks = 0;
    out->_mEstimatedTasks = estimatedTasks;
    return nbl::Result::SUCCESS;
}

void nbl::mc::DownloadManager::_callback(void* userData, Result res, i32 code) {
    DownloadManager* self = static_cast<DownloadManager*>(userData);
    if(res.success()) {
        self->_mComplete++;
        self->_pTracker->pushProgress(static_cast<f64>(self->_mComplete) / static_cast<f64>(nbl::max(self->_mEstimatedTasks, self->_mTasks)));
    }
}
