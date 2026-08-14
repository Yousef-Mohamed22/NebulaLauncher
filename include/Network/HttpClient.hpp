// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file HttpClient.hpp
 * \author jwall64
 * \brief Contains a HttpClient class which can be used to make web requests.
 */

#pragma once

#include <Core/Array.hpp>
#include <Platform/Process.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Pool.hpp>
#include <Platform/Pipe.hpp>
#include <Platform/Mutex.hpp>
#include <Platform/File.hpp>
#include <Platform/PlatformDefines.hpp>
#include <Core/Optional.hpp>
#include <Core/Result.hpp>

#include <functional>


namespace nbl {


enum class HttpClientFlag : u32 {
    REFERENCE_FILE_PATHS = 0b0001,
    CREATE_FILE_PARENTS = 0b0010,
};
typedef u32 HttpClientFlags;


inline HttpClientFlags operator&(HttpClientFlag flag, HttpClientFlags other) {
    return static_cast<u32>(flag) & other;
}

inline HttpClientFlags operator&(HttpClientFlags other, HttpClientFlag flag) {
    return static_cast<u32>(flag) & other;
}

inline HttpClientFlags operator|(HttpClientFlag flag, HttpClientFlags other) {
    return static_cast<u32>(flag) | other;
}

inline HttpClientFlags operator|(HttpClientFlags other, HttpClientFlag flag) {
    return static_cast<u32>(flag) | other;
}


struct HttpHeader {
    StringView name;
    StringView value;

    bool validateName() const {

        static constexpr Array _allowedSpecials = {
            '!', '#', '$', '%', '&', '\'', '*', '+', '-', '.', '^', '_', '`', '|'
        };

        for(char c : name) {
            if(c >= 'a' && c <= 'z') continue;
            if(c >= 'A' && c <= 'Z') continue;
            if(c >= '0' && c <= '9') continue;
            if(_allowedSpecials.contains(c)) continue;
            return false;
        }

        return true;
    }
};

class HttpClient {
public:

    ~HttpClient();

    Result downloadFile(StringView url, StringView file, Optional<std::function_ref<void(Result)>> doneCallback, Span<HttpHeader> headers = {});
    Result makeRequest(StringView url, Writer writer, Optional<std::function_ref<void(Result)>> doneCallback, Span<HttpHeader> headers = {});
    Result makePostRequest(StringView url, StringView body, Writer output, Optional<std::function_ref<void(Result)>> doneCallback, Span<HttpHeader> headers = {});

    void awaitAll(u32* numFailures);

    static Result create(u32 maxRequests, u32 maxConnections, u32 maxRetries, HttpClient* out, HttpClientFlags flags = 0);
    inline static Result create(u32 maxRequests, u32 maxConnections, u32 maxRetries, HttpClient* out, HttpClientFlag flag) {
        return create(maxRequests, maxConnections, maxRetries, out, static_cast<HttpClientFlags>(flag));
    }

private:

    struct _ActiveRequest {

        inline ~_ActiveRequest() {
            callback.~Optional<std::function_ref<void(Result)>>();
            if(download && fileOpened) {
                file.~FileChannel();
            }
        };

        Writer writer;
        union {
            FileChannel file;
            StringView filePath;
        };
        void* handle;
        void* headers;

        Optional<std::function_ref<void(Result)>> callback;
        u32 retryCount;
        bool download;
        bool queued;
        bool fileOpened;
        bool createParents;

        inline void finished(Result res) {
            if(callback.hasValue()) {
                (*callback)(res);
            }
        }
    };

    nbl::Result _setupConnection(StringView url, _ActiveRequest* req, Optional<StringView> postData, Span<HttpHeader> headers);
    nbl::Result _startThread();

    nbl::Pool<_ActiveRequest> _mRequests;
    nbl::Arena _mPaths;
    void* _pHandle;
    Thread _mReaderThread;
    Mutex _mRequestMutex;
    Latch _mAllDone;
    u32 _mMaxRetries;
    u32 _mFailures;
    HttpClientFlags _mFlags;
    bool _bQueuePaused;

};

}
