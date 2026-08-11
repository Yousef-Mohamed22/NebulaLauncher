// Copyright (c) 2026 jwall64. All rights reserved.

/**
 * \file Socket.hpp
 * \author jwall64
 * \brief A socket interface that can be used to create TCP or UDP connections and read/write data over the network.
 */

#pragma once

#include <Core/Types.hpp>
#include <Core/String.hpp>
#include <Core/Result.hpp>

namespace nbl {

/**
 * \brief An enumeration representing a type of socket, either TCP or UDP. This is used when connecting to a remote address to specify the desired protocol for the connection.
 */
enum class SocketType {
    TCP,
    UDP
};

/**
 * \brief A simple socket interface that can be used to create TCP or UDP connections and read/write data over the network.
 */
class Socket {
public:

    /**
     * \brief Default constructor that initializes the socket with no underlying handle.
     */
    ~Socket() {
        close();
    }

    /**
     * \brief Returns a Writer interface for writing data to the socket. This allows for sending data over the network through the socket connection.
     * \return A Writer interface for writing data to the socket.
     */
    Writer writer();

    /**
     * \brief Returns a Reader interface for reading data from the socket. This allows for receiving data over the network through the socket connection.
     * \return A Reader interface for reading data from the socket.
     */
    Reader reader();

    /**
     * \brief Implicit conversion operator to allow a Socket to be used as a Writer or Reader. This allows for convenient use of the socket in contexts where a Writer or Reader is expected, without needing to explicitly call the writer() or reader() methods.
     */
    inline operator Writer() {
        return writer();
    }

    /**
     * \brief Implicit conversion operator to allow a Socket to be used as a Reader. This allows for convenient use of the socket in contexts where a Reader is expected, without needing to explicitly call the reader() method.
     */
    inline operator Reader() {
        return reader();
    }

    /**
     * \brief Closes the socket connection and releases any associated resources. After calling this method, the socket should not be used for reading or writing data.
     */
    void close();

    /**
     * \brief Static method to connect to a remote address using the specified protocol and create a Socket object for the connection. This method establishes a network connection to the specified address and port, and returns a Socket object that can be used to read/write data over that connection.
     * \param address The IPv4 address to connect to, represented as a 32-bit unsigned integer in network byte order.
     * \param port The port number to connect to, represented as a 16-bit unsigned integer in host byte order.
     * \param type The type of socket to create, either TCP or UDP.
     * \param outSocket Pointer to a variable that will receive the connected Socket object if the connection is successful.
     * \return A Result indicating success or failure of the connection attempt. If the connection is successful, it returns Result::SUCCESS and sets outSocket to the connected Socket object. If there is an error during the connection attempt, it returns an appropriate error code.
     */
    static Result connectIPv4(u32 address, u16 port, SocketType type, Socket* outSocket);

private:
    void* _mHandle;

    friend class ServerSocket;

};


class ServerSocket {
public:

    Result accept(Socket* client);

    bool isClosed();
    void close();

    static Result listen(u16 port, SocketType type, ServerSocket* outSocket);
private:

    void* _mHandle;

};



}
