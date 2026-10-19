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
#include "Launch/ProcessHandle.hpp"

#include <QRegularExpression>

#include <Platform/Logging.hpp>
#include <Core/Result.hpp>

#define BACKLOG_SIZE 1024 * 1024 // 1 MiB

namespace {

i32 _threadProc(void* userData) {

    nbl::frontend::ProcessHandle* handle = static_cast<nbl::frontend::ProcessHandle*>(userData);
    QString currentLine = "";

    static const QRegularExpression ansiRegex("\x1B\\[[0-9;]*[a-zA-Z]");

    nbl::Reader reader = handle->tracker()->handle()->stdoutReader();
    while(true) {

        char copyBuffer[4096];
        usize bytesRead = 0;
        nbl::Result rc = reader.read(copyBuffer, 4096, &bytesRead);
        if(rc.error()) {
            return 1;
        }

        for(usize i = 0 ; i < bytesRead ; i++) {
            char c = copyBuffer[i];
            if(c == '\n') {
                handle->appendLine(currentLine.remove(ansiRegex));
                currentLine.resizeForOverwrite(0);
            } else {
                currentLine += c;
            }
        }

        if(rc == nbl::Result::END_OF_STREAM) {
            break;
        }
    }
    return 0;
}

}


nbl::Result nbl::frontend::ProcessHandle::initLogging() {

    NBL_CHECK(nbl::Thread::create(_threadProc, this, &_mWatcherThread));
    return nbl::Result::SUCCESS;

}

