#include <Network/HttpServer.hpp>
#include <Platform/Socket.hpp>
#include <Platform/Logging.hpp>
#include <Core/Stream.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

void nbl::OAuthCallbackListener::close() {
    _mListenThread.join(nullptr);
}



i32 nbl::OAuthCallbackListener::_threadProc(void* userData) {

    OAuthCallbackListener* self = static_cast<OAuthCallbackListener*>(userData);

    ArenaFrame frame(Arena::threadLocal());
    StringBuilder builder(&frame);

    NBL_CHECK(OAuthCallbackListener::listen(self->_mPort, &builder));

    StringView path;
    NBL_CHECK(builder.buildNullTerminated(&path));
    NBL_CHECK(self->_pfnHandler(path, self->_pUserData));

    return 0;
}

nbl::Result nbl::OAuthCallbackListener::listen(u16 port, StringBuilder *output) {

    constexpr StringView okResponse = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "\r\n"
        "You may now close this page and return to Nebula";
    
    constexpr StringView clientErrorResponse = 
        "HTTP/1.1 400 Bad Request\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    constexpr StringView errorResponse = 
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Connection: close\r\n"
        "\r\n";

    bool succeeded = false;

    nbl::ServerSocket socket{};
    NBL_CHECK(nbl::ServerSocket::listen(port, SocketType::TCP, &socket));

    while(!succeeded) {

        Socket client;
        nbl::Result res = socket.accept(&client);

        if(res.error()) {
            if(socket.isClosed()) {
                break;
            } else {
                nbl::log(nbl::LogLevel::WARN, "An error occurred while accepting a client!");
                continue;
            }
        }

        u8 buffer[4096];
        char lastRead = 0;
        BufferedReader reader(client.reader(), buffer, 4096);

        auto _next = [&]() -> Result {
            return reader.readAll(&lastRead, 1);
        };

        auto _reject = [&]() -> void {
            client.writer().writeString(clientErrorResponse);
            client.close();
        };

        u8 state = 0;

        bool done = false;

        while(!done) {
            res = _next();
            if(res != Result::SUCCESS) {
                client.writer().writeString(res == Result::END_OF_STREAM ? clientErrorResponse : errorResponse);
                client.close();
                done = true;
                break;
            }

            switch(state) {
                case 0:
                    if(lastRead == 'G') { state++; } else { _reject(); }
                    break;
                case 1: 
                    if(lastRead == 'E') { state++; } else { _reject(); }
                    break;
                case 2: 
                    if(lastRead == 'T') { state++; } else { _reject(); }
                    break;
                case 3: 
                    if(lastRead == ' ') { state++; } else { _reject(); }
                    break;
                case 4:
                    if(lastRead == ' ') {
                        state++;

                        if(output->length() == 0) {
                            client.writer().writeString(clientErrorResponse);
                            client.close();
                            done = true;
                            break;
                        }

                        if(res.error()) {
                            client.writer().writeString(errorResponse);
                            client.close();
                            done = true;
                            break;
                        }

                        client.writer().writeString(okResponse);
                        client.close();

                        succeeded = true;
                        done = true;
                        break;
                    };
                    output->append(lastRead);
                    break;
            
                case 5:
                    client.writer().writeString(res == Result::END_OF_STREAM ? clientErrorResponse : errorResponse);
                    client.close();
                    done = true;
                    break;
            }
        }
    }

    socket.close();
    return succeeded ? 0 : -1;
}


nbl::Result nbl::OAuthCallbackListener::listenAsync(u16 port, Result(*pfnHandler)(StringView, void*), void* userData, OAuthCallbackListener* outServer) {

    NBL_CHECK(nbl::Thread::create(_threadProc, outServer, &outServer->_mListenThread));

    outServer->_pUserData = userData;
    outServer->_pfnHandler = pfnHandler;

    return nbl::Result::SUCCESS;
}
