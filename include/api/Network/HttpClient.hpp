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
#include <Platform/Pipe.hpp>
#include <Platform/Mutex.hpp>

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



class HttpReader;
class HttpClient {
public:

    HttpClient() = default;
    ~HttpClient() { close(); }

    Result makeRequest(StringView url, HttpReader* responseReader, Span<HttpHeader> headers = {});

    void close();

    static Result create(HttpClient* outClient);

private:
 
    friend class HttpReader;

    static i32 _threadProc(void* userData);

    void* _pHandle;

    nbl::Mutex _mRequestMutex;
    nbl::Thread _mThread;

};


class HttpReader {
public:

    HttpReader() = default;
    ~HttpReader() { close(); }

    inline operator Reader() { return reader(); }

    inline Reader reader() { 
        return {
            .impl = this,
            .pfnRead = _read
        }; 
    }

    void close();

private:

    friend class HttpClient;

    static Result _read(void* impl, void* buffer, usize bufLen, usize* bytesRead);

    void* _pHandle;
    HttpClient* _pParent;
    nbl::Pipe _mPipe;
    nbl::Result _mError;
};

}
