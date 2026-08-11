#include <Network/HttpServer.hpp>
#include <Platform/Socket.hpp>
#include <Platform/Logging.hpp>
#include <Core/Stream.hpp>
#include <Platform/Arena.hpp>
#include <Platform/StringBuilder.hpp>

void nbl::HttpServer::close() {



}



i32 nbl::HttpServer::_threadProc(void* userData) {

    HttpServer* self = static_cast<HttpServer*>(userData);

    constexpr StringView okResponse = 
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    constexpr StringView clientErrorResponse = 
        "HTTP/1.1 400 Bad Request\r\n"
        "Connection: close\r\n"
        "\r\n";
    
    constexpr StringView errorResponse = 
        "HTTP/1.1 500 Internal Server Error\r\n"
        "Connection: close\r\n"
        "\r\n";

    while(true) {

        Socket client;
        nbl::Result res = self->_mSocket.accept(&client);

        if(res.error()) {
            if(self->_mSocket.isClosed()) {
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

        ArenaFrame frame(Arena::threadLocal());
        StringBuilder pathBuilder(&frame);
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

                        StringView path;
                        res = pathBuilder.buildNullTerminated(&path);
                        if(res.error()) break;

                        if(path.length == 0) {
                            client.writer().writeString(clientErrorResponse);
                            client.close();
                            done = true;
                            break;
                        }

                        res = self->_pfnHandler(path, self->_pUserData);
                        if(res.error()) break;

                        client.writer().writeString(okResponse);
                        client.close();

                        done = true;
                    };
                    pathBuilder.append(lastRead);
                    break;
            
                case 5:
                    client.writer().writeString(res == Result::END_OF_STREAM ? clientErrorResponse : errorResponse);
                    client.close();
                    done = true;
                    break;
            }
        }
    }

    return 0;
}


nbl::Result nbl::HttpServer::create(u16 port, Result(*pfnHandler)(StringView, void*), void* userData, HttpServer* outServer) {

    NBL_CHECK(nbl::ServerSocket::listen(port, SocketType::TCP, &outServer->_mSocket));
    NBL_CHECK(nbl::Thread::create(_threadProc, outServer, nullptr));

    outServer->_pUserData = userData;
    outServer->_pfnHandler = pfnHandler;

    return nbl::Result::SUCCESS;
}
