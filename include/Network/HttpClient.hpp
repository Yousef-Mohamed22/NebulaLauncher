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

    void awaitAll(u32* numFailures);

    static Result create(u32 maxRequests, u32 maxConnections, u32 maxRetries, HttpClient* out);

private:

    struct _ActiveRequest {
        Writer writer;
        FileChannel file;
        void* handle;
        void* headers;

        Optional<std::function_ref<void(Result)>> callback;
        u32 retryCount;
        bool download;
        bool queued;

        inline void finished(Result res) {
            if(callback.hasValue()) {
                (*callback)(res);
            }
        }
    };

    nbl::Result _setupConnection(StringView url, _ActiveRequest* req, Span<HttpHeader> headers = {});
    nbl::Result _startThread();


    nbl::Pool<_ActiveRequest> _mRequests;
    void* _pHandle;
    Thread _mReaderThread;
    Mutex _mRequestMutex;
    Latch _mAllDone;
    u32 _mMaxRetries;
    u32 _mFailures;

};

}
