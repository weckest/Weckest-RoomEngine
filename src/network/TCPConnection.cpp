//
// Created by Weckest on 2026-09-28.
//

#include "TCPConnection.h"

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "hardware.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "services/ModuleService.h"

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <fcntl.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

class RoomEngine;
extern RoomEngine* g_engine;

TcpConnection::TcpConnection(TcpSocket socket)
    : socket(socket),
      connected(true),
      lastHeartbeat(std::chrono::steady_clock::now())
{
    if (!SetNonBlocking())
    {
        std::cerr << "Failed to set client socket " << socket
                  << " to non-blocking mode.\n";
        connected = false;
        return;
    }

    std::cout << "TcpConnection created for socket " << socket << '\n';
}

TcpConnection::~TcpConnection()
{
    Close();
}

bool TcpConnection::IsConnected() const
{
    return connected;
}

TcpSocket TcpConnection::GetSocket() const
{
    return socket;
}

bool TcpConnection::SetNonBlocking()
{
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(static_cast<SOCKET>(socket), FIONBIO, &mode) != 0)
    {
        std::cerr << "ioctlsocket(FIONBIO) failed for socket " << socket
                  << ", error " << WSAGetLastError() << '\n';
        return false;
    }
#else
    const int currentFlags = fcntl(socket, F_GETFL, 0);
    if (currentFlags == -1)
    {
        std::cerr << "fcntl(F_GETFL) failed for socket " << socket
                  << ", errno " << errno << " (" << std::strerror(errno) << ")\n";
        return false;
    }

    if (fcntl(socket, F_SETFL, currentFlags | O_NONBLOCK) == -1)
    {
        std::cerr << "fcntl(O_NONBLOCK) failed for socket " << socket
                  << ", errno " << errno << " (" << std::strerror(errno) << ")\n";
        return false;
    }
#endif

    return true;
}

bool TcpConnection::Update(std::vector<ModuleMessage>& messages)
{
    if (!connected)
        return false;

    uint8_t buffer[1024];

    while (true)
    {
#ifdef _WIN32
        const int bytesReceived = recv(
            static_cast<SOCKET>(socket),
            reinterpret_cast<char*>(buffer),
            static_cast<int>(sizeof(buffer)),
            0
        );
#else
        const ssize_t bytesReceived = recv(
            socket,
            buffer,
            sizeof(buffer),
            0
        );
#endif

        if (bytesReceived > 0)
        {
            receiveBuffer.insert(
                receiveBuffer.end(),
                buffer,
                buffer + bytesReceived
            );
            continue;
        }

        // recv() returning zero means the peer closed the connection.
        if (bytesReceived == 0)
        {
            std::cout << "Client closed connection: " << socket << '\n';
            Close();
            return false;
        }

#ifdef _WIN32
        const int error = WSAGetLastError();

        if (error == WSAEWOULDBLOCK)
            break;

        if (error == WSAECONNRESET ||
            error == WSAECONNABORTED ||
            error == WSAENOTCONN)
        {
            std::cout << "Connection lost for socket " << socket
                      << " (Winsock error " << error << ")\n";
            Close();
            return false;
        }
#else
        const int error = errno;

        if (error == EWOULDBLOCK || error == EAGAIN)
            break;

        if (error == ECONNRESET ||
            error == ECONNABORTED ||
            error == ENOTCONN ||
            error == EBADF)
        {
            std::cout << "Connection lost for socket " << socket
                      << " (errno " << error << ": " << std::strerror(error) << ")\n";
            Close();
            return false;
        }

        if (error == EINTR)
            continue;
#endif

        std::cerr << "recv() failed for socket " << socket
#ifdef _WIN32
                  << " with Winsock error " << error
#else
                  << " with errno " << error << " (" << std::strerror(error) << ")"
#endif
                  << '\n';
        Close();
        return false;
    }

    if (!ProcessReceiveBuffer(messages))
    {
        Close();
        return false;
    }

    if (!CheckHeartbeatTimeout())
        return false;

    return true;
}

bool TcpConnection::ProcessReceiveBuffer(std::vector<ModuleMessage>& messages)
{
    while (true)
    {
        // A frame starts with a four-byte, big-endian payload length.
        if (receiveBuffer.size() < 4)
            return true;

        const uint32_t messageSize =
            (static_cast<uint32_t>(receiveBuffer[0]) << 24) |
            (static_cast<uint32_t>(receiveBuffer[1]) << 16) |
            (static_cast<uint32_t>(receiveBuffer[2]) << 8) |
            static_cast<uint32_t>(receiveBuffer[3]);

        if (messageSize == 0 || messageSize > MAX_MESSAGE_SIZE)
        {
            std::cerr << "Invalid message size from socket "
                      << socket << ": " << messageSize << '\n';
            return false;
        }

        const size_t totalMessageSize = 4 + static_cast<size_t>(messageSize);
        if (receiveBuffer.size() < totalMessageSize)
            return true; // Wait for the rest of this frame.

        ModuleMessage message = ModuleMessage_init_zero;
        if (!DecodeMessage(receiveBuffer.data() + 4, messageSize, message))
            return false;

        if (message.which_payload == ModuleMessage_heartbeat_tag)
            HandleHeartbeat(message);

        messages.push_back(message);

        receiveBuffer.erase(
            receiveBuffer.begin(),
            receiveBuffer.begin() + static_cast<std::ptrdiff_t>(totalMessageSize)
        );
    }
}

bool TcpConnection::DecodeMessage(
    const uint8_t* buffer,
    size_t size,
    ModuleMessage& message)
{
    pb_istream_t stream = pb_istream_from_buffer(buffer, size);

    if (!pb_decode(&stream, ModuleMessage_fields, &message))
    {
        std::cerr << "Failed to decode protobuf on socket " << socket
                  << ": " << PB_GET_ERROR(&stream) << '\n';
        return false;
    }

    return true;
}

bool TcpConnection::Send(const ModuleMessage& message)
{
    if (!connected)
    {
        std::cerr << "Cannot send message: socket " << socket
                  << " is not connected.\n";
        return false;
    }

    uint8_t buffer[MAX_MESSAGE_SIZE];
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, ModuleMessage_fields, &message))
    {
        std::cerr << "Failed to encode protobuf: " << PB_GET_ERROR(&stream) << '\n';
        return false;
    }

    const uint32_t messageSize = static_cast<uint32_t>(stream.bytes_written);
    const uint8_t header[4] = {
        static_cast<uint8_t>((messageSize >> 24) & 0xFF),
        static_cast<uint8_t>((messageSize >> 16) & 0xFF),
        static_cast<uint8_t>((messageSize >> 8) & 0xFF),
        static_cast<uint8_t>(messageSize & 0xFF)
    };

    if (!SendAll(header, sizeof(header)))
    {
        std::cerr << "Failed to send message header on socket " << socket << '\n';
        return false;
    }

    if (!SendAll(buffer, messageSize))
    {
        std::cerr << "Failed to send protobuf message on socket " << socket << '\n';
        return false;
    }

    return true;
}

bool TcpConnection::SendAll(const uint8_t* buffer, size_t size)
{
    size_t totalSent = 0;

    while (totalSent < size)
    {
#ifdef _WIN32
        const int remaining = static_cast<int>(size - totalSent);
        const int bytesSent = send(
            static_cast<SOCKET>(socket),
            reinterpret_cast<const char*>(buffer + totalSent),
            remaining,
            0
        );

        if (bytesSent == SOCKET_ERROR)
        {
            const int error = WSAGetLastError();
            std::cerr << "send() failed on socket " << socket
                      << " with Winsock error " << error << '\n';
            return false;
        }
#else
        const ssize_t bytesSent = send(
            socket,
            buffer + totalSent,
            size - totalSent,
#ifdef MSG_NOSIGNAL
            MSG_NOSIGNAL
#else
            0
#endif
        );

        if (bytesSent < 0)
        {
            const int error = errno;
            if (error == EINTR)
                continue;

            std::cerr << "send() failed on socket " << socket
                      << " with errno " << error << " (" << std::strerror(error) << ")\n";
            return false;
        }
#endif

        if (bytesSent == 0)
            return false;

        totalSent += static_cast<size_t>(bytesSent);
    }

    return true;
}

void TcpConnection::HandleHeartbeat(const ModuleMessage& message)
{
    lastHeartbeat = std::chrono::steady_clock::now();
    (void)message;
}

bool TcpConnection::CheckHeartbeatTimeout()
{
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat);

    if (elapsed >= HEARTBEAT_TIMEOUT)
    {
        std::cout << "Heartbeat timeout for socket " << socket
                  << " (" << elapsed.count() << " seconds)\n";
        Close();
        return false;
    }

    return true;
}

void TcpConnection::Close()
{
    if (!connected)
        return;

    std::cout << "Closing TCP connection: " << socket << '\n';

#ifdef _WIN32
    shutdown(static_cast<SOCKET>(socket), SD_BOTH);
    closesocket(static_cast<SOCKET>(socket));
#else
    shutdown(socket, SHUT_RDWR);
    close(socket);
#endif

    connected = false;
    socket = -1;
    receiveBuffer.clear();
}
