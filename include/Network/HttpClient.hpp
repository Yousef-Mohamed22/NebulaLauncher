// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file HttpClient.hpp
 * \author jwall64
 * \brief Contains a HttpClient class which can be used to make web requests.
 */

#pragma once

#include <Core/Array.hpp>
#include <Core/Either.hpp>
#include <Core/Optional.hpp>
#include <Core/Result.hpp>
#include <Platform/Process.hpp>
#include <Platform/Arena.hpp>
#include <Platform/Pool.hpp>
#include <Platform/Pipe.hpp>
#include <Platform/Mutex.hpp>
#include <Platform/File.hpp>
#include <Platform/PlatformDefines.hpp>
#include <Platform/StringBuilder.hpp>


namespace nbl {


enum class HttpClientFlag : u32 {
    REFERENCE_FILE_PATHS = 0b0001,
    CREATE_FILE_PARENTS = 0b0010,
};
typedef u32 HttpClientFlags;

inline HttpClientFlags operator&(HttpClientFlag flag, HttpClientFlag other) {
    return static_cast<u8>(flag) & static_cast<u8>(other);
}

inline HttpClientFlags operator&(HttpClientFlags existing, HttpClientFlag flag) {
    return static_cast<u8>(flag) & existing;
}

inline HttpClientFlags operator&(HttpClientFlag flag, HttpClientFlags existing) {
    return static_cast<u8>(flag) & existing;
}

inline HttpClientFlags operator&=(HttpClientFlag& flag, HttpClientFlag other) {
    return static_cast<u8>(flag) & static_cast<u8>(other);
}

inline HttpClientFlags operator&=(HttpClientFlags& existing, HttpClientFlag flag) {
    return static_cast<u8>(flag) & existing;
}

inline HttpClientFlags operator&=(HttpClientFlag& flag, HttpClientFlags existing) {
    return static_cast<u8>(flag) & existing;
}

inline HttpClientFlags operator|(HttpClientFlag flag, HttpClientFlag other) {
    return static_cast<u8>(flag) | static_cast<u8>(other);
}

inline HttpClientFlags operator|(HttpClientFlag flag, HttpClientFlags existing) {
    return static_cast<u8>(flag) | existing;
}

inline HttpClientFlags operator|(HttpClientFlags existing, HttpClientFlag flag) {
    return static_cast<u8>(flag) | existing;
}

inline HttpClientFlags operator|=(HttpClientFlag& flag, HttpClientFlag other) {
    return static_cast<u8>(flag) | static_cast<u8>(other);
}

inline HttpClientFlags operator|=(HttpClientFlag& flag, HttpClientFlags existing) {
    return static_cast<u8>(flag) | existing;
}

inline HttpClientFlags operator|=(HttpClientFlags& existing, HttpClientFlag flag) {
    return static_cast<u8>(flag) | existing;
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

struct HttpRequestOptions {
    void(*doneCallback)(void*, Result, i32);
    void* userData;
    Span<HttpHeader> headers;
    u64 ifModifiedSince;
    bool markExecutable;
};

struct UrlEncode {
    StringView string;
};

Result toString(const UrlEncode& encode, Writer writer);

class HttpClient {
public:

    ~HttpClient();

    Result downloadFile(StringView url, StringView file, HttpRequestOptions options = {});
    Result makeRequest(StringView url, Writer writer, HttpRequestOptions options = {});
    Result makePostRequest(StringView url, StringView body, Writer output, HttpRequestOptions options = {});

    void awaitAll(u32* numFailures);

    static Result create(u32 maxRequests, u32 maxConnections, u32 maxRetries, u32 maxOpenFiles, HttpClient* out, HttpClientFlags flags = 0);
    inline static Result create(u32 maxRequests, u32 maxConnections, u32 maxRetries, u32 maxOpenFiles, HttpClient* out, HttpClientFlag flag) {
        return create(maxRequests, maxConnections, maxRetries, maxOpenFiles, out, static_cast<HttpClientFlags>(flag));
    }

private:

    struct _ActiveRequest {

        Writer writer;
        Either<FileChannel, StringView> file; 
        void* handle;
        void* headers;

        void(*doneCallback)(void*, Result, i32);
        void* userData;

        u32 retryCount;
        bool download;
        bool queued;
        bool createParents;
        bool markExecutable;

        inline bool fileOpened() {
            return file.hasLeft();
        }

        inline void finished(Result res, i32 statusCode) {
            if(doneCallback) {
                doneCallback(userData, res, statusCode);
            }
        }
    };

    nbl::Result _setupConnection(StringView url, _ActiveRequest* req, Optional<StringView> postData, const HttpRequestOptions& options);
    nbl::Result _startThread();

    nbl::Pool<_ActiveRequest> _mRequests;
    nbl::Arena _mPaths;
    void* _pHandle;
    Thread _mReaderThread;
    Mutex _mRequestMutex;
    Latch _mAllDone;
    HttpClientFlags _mFlags;
    u32 _mMaxRetries;
    u32 _mFailures;
    u32 _mMaxOpenFiles;
    std::atomic<u32> _mOpenFiles;
    std::atomic<u32> _mQueuedRequests;
    bool _bQueuePaused;

};

}
