#pragma once

#include <Core/String.hpp>
#include <Platform/Socket.hpp>
#include <Platform/Process.hpp>

namespace nbl {


class HttpServer {
public:

    ~HttpServer() {
        close();
    }

    void close();

    static Result create(u16 port, Result(*pfnHandler)(StringView, void*), void* userData, HttpServer* outServer);

private:

    static i32 _threadProc(void* userData);

    ServerSocket _mSocket;
    Thread _mListenThread;

    Result(*_pfnHandler)(StringView path, void* userData);
    void* _pUserData;

};


}
