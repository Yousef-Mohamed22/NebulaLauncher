#include "NblProcessHandle.hpp"
#include <qobject.h>
#include <QRegularExpression>

#include <Platform/Logging.hpp>
#include <Core/Result.hpp>

#define BACKLOG_SIZE 1024 * 1024 // 1 MiB

namespace {

i32 _threadProc(void* userData) {

    NblProcessHandle* handle = static_cast<NblProcessHandle*>(userData);
    QString currentLine = "";

    static const QRegularExpression ansiRegex("\x1B\\[[0-9;]*[a-zA-Z]");

    nbl::Reader reader = handle->handle()->stdoutReader();
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


nbl::Result NblProcessHandle::initLogging() {

    NBL_CHECK(nbl::Thread::create(_threadProc, this, &_mWatcherThread));
    return nbl::Result::SUCCESS;

}

