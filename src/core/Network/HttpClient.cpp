// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file HttpClient.cpp
 * \author jwall64
 * \brief Implementation of Network/HttpClient.hpp.
 */

#include <Network/HttpClient.hpp>

#include <Core/Result.hpp>
#include <Platform/StringBuilder.hpp>
#include <Platform/Pipe.hpp>

#include <curl/curl.h>
#include <curl/multi.h>


namespace {

constexpr const char* _USER_AGENT = "Nebula/0.1.0 curl/8.20.0";

bool _sInitialized = false;

usize _writeCallback(char* ptr, usize size, usize nmemb, void* userData) {

    if(nmemb == 0) return 0;

    // Hack: uninitialized pipe still produces writer with correct pfnWrite
    nbl::Writer writer = nbl::Pipe().writer();
    writer.impl = userData;

    nbl::Result res = writer.writeAll(ptr, nmemb);
    if(res.error()) {
        return 0;
    }

    return nmemb;
}

}

void nbl::HttpReader::close() {

    if(_pHandle && _pParent && _pParent->_pHandle) {
        CURL* handle = static_cast<CURL*>(_pHandle);
        CURLM* multi = static_cast<CURLM*>(_pParent->_pHandle);

        _pParent->_mRequestMutex.lock();
        curl_multi_wakeup(multi);

        curl_multi_remove_handle(multi, handle);

        _pParent->_mRequestMutex.unlock();
        _pHandle = nullptr;
    }

    _mPipe.closeWriter();
}

nbl::Result nbl::HttpReader::_read(void* impl, void* buffer, usize bufLen, usize* bytesRead) {

    HttpReader* reader = static_cast<HttpReader*>(impl);
    nbl::Result res = reader->_mPipe.reader().read(buffer, bufLen, bytesRead);
    if(reader->_mError.error()) {
        return reader->_mError;
    }
    return res;
}


nbl::Result nbl::HttpClient::makeRequest(StringView url, HttpReader* outReader, Span<HttpHeader> headers) {

    // TODO: Proper synchronization when the thread is alive
    CURLM* multiHandle = static_cast<CURLM*>(_pHandle);
    CURL* easyHandle = curl_easy_init();

    nbl::ArenaFrame frame(nbl::Arena::threadLocal());

    StringView urlCopy;
    NBL_CHECK(nbl::StringBuilder::nullTerminate(url, &urlCopy, &frame));

    NBL_CHECK(nbl::Pipe::create(&outReader->_mPipe));

    curl_easy_setopt(easyHandle, CURLOPT_USERAGENT, _USER_AGENT);
    curl_easy_setopt(easyHandle, CURLOPT_WRITEFUNCTION, _writeCallback);
    curl_easy_setopt(easyHandle, CURLOPT_WRITEDATA, outReader->_mPipe.writer().impl);
    curl_easy_setopt(easyHandle, CURLOPT_PRIVATE, outReader);

    if(headers.length > 0) {
        curl_slist* list = nullptr;
    
        for(const HttpHeader& header : headers) { 
            if(!header.validateName())
                return nbl::error::ILLEGAL_ARGUMENT;

            StringView headerStr;
            NBL_CHECK(nbl::StringBuilder(&frame)
                .append(header.name)
                .append(": ")
                .append(header.value)
                .buildNullTerminated(&headerStr));

            list = curl_slist_append(list, headerStr.data);
        }

        curl_easy_setopt(easyHandle, CURLOPT_HTTPHEADER, list);
    }
    curl_easy_setopt(easyHandle, CURLOPT_URL, urlCopy.data);

    _mRequestMutex.lock();
    curl_multi_wakeup(multiHandle);

    CURLMcode rc = curl_multi_add_handle(multiHandle, easyHandle);
    if(rc != 0) {
        return nbl::error::IO_ERROR;
    }
    _mRequestMutex.unlock();

    if(!_mThread.isRunning()) {
        _mThread.~Thread();
        nbl::Thread::create(_threadProc, this, &_mThread);
    }
    
    outReader->_pParent = this;
    outReader->_pHandle = easyHandle;
    return nbl::Result::SUCCESS;
}

nbl::Result nbl::HttpClient::create(nbl::HttpClient* outClient) {

    if(!_sInitialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        _sInitialized = true;
    }

    CURLM* multiHandle = curl_multi_init();
    if(multiHandle == nullptr) return nbl::error::IO_ERROR;

    NBL_CHECK(nbl::Mutex::create(&outClient->_mRequestMutex));

    outClient->_pHandle = multiHandle;
    return nbl::Result::SUCCESS;
}

void nbl::HttpClient::close() {

    if(_pHandle) {
        _mRequestMutex.lock();
        curl_multi_wakeup(_pHandle);

        CURL** handles = curl_multi_get_handles(_pHandle);
        CURL* current = handles[0];
        usize index = 0;
        while(current != nullptr) {
        
            HttpReader* reader;
            curl_easy_getinfo(current, CURLINFO_PRIVATE, &reader);

            reader->_mError = nbl::error::ILLEGAL_STATE;
            reader->close();

            index++;
            current = handles[index];
        }

        _mThread.join(nullptr);
        _mRequestMutex.unlock();

        CURLM* multi = static_cast<CURLM*>(_pHandle);
        curl_multi_cleanup(multi);
        _pHandle = nullptr;
    }
}


i32 nbl::HttpClient::_threadProc(void* userData) {
    
    nbl::HttpClient* client = static_cast<nbl::HttpClient*>(userData);
    CURLM* handle = static_cast<CURLM*>(client->_pHandle);

    int runningRequests = 0;
    do {
        if(curl_multi_perform(handle, &runningRequests) != CURLM_OK) {
            return 1;
        }
        if(curl_multi_poll(handle, nullptr, 0, 0, nullptr) != CURLM_OK) {
            return 2;
        }

        client->_mRequestMutex.lock();
        client->_mRequestMutex.unlock();

        CURLMsg* msg;
        int msgsLeft;
        while((msg = curl_multi_info_read(handle, &msgsLeft)) != nullptr) {
            if(msg->msg == CURLMSG_DONE) {
                CURL* easy = msg->easy_handle;
                HttpReader* reader;
                curl_easy_getinfo(easy, CURLINFO_PRIVATE, &reader);

                if(msg->data.result != CURLE_OK) {
                    nbl::log(nbl::LogLevel::ERROR, "HTTP connection failed! ", curl_easy_strerror(msg->data.result));
                    reader->_mError = nbl::error::IO_ERROR;
                }
                reader->close();
            }
        }

    } while(runningRequests > 0);

    return 0;
}
