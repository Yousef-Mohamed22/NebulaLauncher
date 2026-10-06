#pragma once

#include <Core/String.hpp>
#include <Platform/Socket.hpp>
#include <Platform/Process.hpp>
#include <Platform/StringBuilder.hpp>

namespace nbl {


class OAuthCallbackListener {
public:

    ~OAuthCallbackListener() {
        close();
    }

    void close();

    static Result listenAsync(u16 port, Result(*pfnHandler)(StringView, void*), void* userData, OAuthCallbackListener* outListener);
    static Result listen(u16 port, StringBuilder* output);

private:

    static i32 _threadProc(void* userData);

    u16 _mPort;
    Thread _mListenThread;

    Result(*_pfnHandler)(StringView path, void* userData);
    void* _pUserData;

};


}
