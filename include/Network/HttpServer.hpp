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


/**
 * \file Network/HttpServer.hpp
 * \author jwall64
 * \brief Minimal HTTP server designed specifically to receive OAuth callbacks.
 */

#pragma once

#include <Core/String.hpp>
#include <Platform/Socket.hpp>
#include <Platform/Process.hpp>
#include <Utility/StringBuilder.hpp>

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
