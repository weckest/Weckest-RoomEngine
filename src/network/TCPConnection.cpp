//
// Created by Weckest on 2026-09-28.
//

#include "TCPConnection.h"

#include <iostream>
#include <cstring>

#include <iostream>
#include <vector>
#include <chrono>

#include "hardware.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "services/ModuleService.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#endif

class RoomEngine;
extern RoomEngine* g_engine;

TcpConnection::TcpConnection(int socket)
    : socket(socket),
      connected(true),
      lastHeartbeat(std::chrono::steady_clock::now())
{
    if (!SetNonBlocking())
    {
        std::cerr << "Failed to set client socket " << socket << " to non-blocking mode.\n";

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


int TcpConnection::GetSocket() const
{
    return socket;
}


bool TcpConnection::SetNonBlocking()
{
#ifdef _WIN32

    u_long mode = 1;

    if (ioctlsocket(static_cast<SOCKET>(socket), FIONBIO, &mode) != 0)
    {
        std::cerr << "ioctlsocket(FIONBIO) failed for socket " << socket << '\n';
        return false;
    }

    return true;

#else

    return false;

#endif
}


bool TcpConnection::Update(std::vector<ModuleMessage>& messages)
{
    if (!connected)
    {
        return false;
    }

    /*
     * Read everything currently available.
     *
     * Because the socket is non-blocking, recv() will return
     * WSAEWOULDBLOCK when there is currently no more data.
     */
    uint8_t buffer[1024];

    while (true)
    {
        int bytesReceived = recv(
            static_cast<SOCKET>(socket),
            reinterpret_cast<char*>(buffer),
            sizeof(buffer),
            0
        );

        if (bytesReceived > 0)
        {
            receiveBuffer.insert(
                receiveBuffer.end(),
                buffer,
                buffer + bytesReceived
            );

            continue;
        }

        /*
         * recv() returning 0 means the remote side has
         * gracefully closed the TCP connection.
         */
        if (bytesReceived == 0)
        {
            std::cout
                << "Client closed connection: "
                << socket
                << '\n';

            Close();

            return false;
        }

        /*
         * Negative result.
         */
        int error = WSAGetLastError();

        if (error == WSAEWOULDBLOCK)
        {
            /*
             * No more data is currently available.
             * This is normal for a non-blocking socket.
             */
            break;
        }

        if (error == WSAECONNRESET ||
            error == WSAECONNABORTED ||
            error == WSAENOTCONN)
        {
            std::cout
                << "Connection lost for socket "
                << socket
                << '\n';

            Close();

            return false;
        }

        std::cerr
            << "recv() failed for socket "
            << socket
            << " with error "
            << error
            << '\n';

        Close();

        return false;
    }

    /*
     * We may have received:
     *
     *  - half a message
     *  - exactly one message
     *  - several messages
     *
     * Process as many complete messages as possible.
     */
    if (!ProcessReceiveBuffer(messages))
    {
        Close();
        return false;
    }

    if (!CheckHeartbeatTimeout())
    {
        return false;
    }

    return true;

}

bool TcpConnection::ProcessReceiveBuffer(std::vector<ModuleMessage>& messages)
{
    while (true)
    {
        /*
         * We need at least the 4-byte message length.
         */
        if (receiveBuffer.size() < 4)
        {
            return true;
        }

        /*
         * Decode the 4-byte big-endian size.
         */
        uint32_t messageSize =
            (static_cast<uint32_t>(receiveBuffer[0]) << 24) |
            (static_cast<uint32_t>(receiveBuffer[1]) << 16) |
            (static_cast<uint32_t>(receiveBuffer[2]) << 8) |
            static_cast<uint32_t>(receiveBuffer[3]);

        /*
         * Validate the message size before allocating or decoding.
         */
        if (messageSize == 0 || messageSize > MAX_MESSAGE_SIZE)
        {
            std::cerr
                << "Invalid message size from socket "
                << socket << ": " << messageSize << '\n';

            return false;
        }

        /*
         * Do we have the complete protobuf payload yet?
         */
        const size_t totalMessageSize = 4 + static_cast<size_t>(messageSize);

        if (receiveBuffer.size() < totalMessageSize)
        {
            /*
             * Partial TCP message.
             *
             * Keep the data and wait for another recv().
             */
            return true;
        }

        /*
         * We have a complete framed message.
         */
        ModuleMessage message = ModuleMessage_init_zero;

        if (!DecodeMessage(receiveBuffer.data() + 4, messageSize, message))
        {
            return false;
        }

        if (message.which_payload ==
            ModuleMessage_heartbeat_tag)
        {
            HandleHeartbeat(message);
        }

        messages.push_back(message);

        /*
         * Remove this complete message from the receive buffer.
         */
        receiveBuffer.erase(
            receiveBuffer.begin(),
            receiveBuffer.begin() + totalMessageSize
        );

        /*
         * Loop again in case multiple messages arrived
         * in the same TCP packet.
         */
    }
}


bool TcpConnection::DecodeMessage(const uint8_t* buffer, size_t size, ModuleMessage& message)
{
    pb_istream_t stream = pb_istream_from_buffer(buffer, size);

    if (!pb_decode(&stream, ModuleMessage_fields, &message))
    {
        std::cerr
            << "Failed to decode protobuf on socket "
            << socket << ": " << PB_GET_ERROR(&stream) << '\n';

        return false;
    }

    // std::cout
    //     << "Received protobuf message ("
    //     << size << " bytes) on socket " << socket << '\n';

    return true;
}


bool TcpConnection::Send(const ModuleMessage& message)
{
    if (!connected)
    {
        std::cerr << "Cannot send message: socket " << socket << " is not connected.\n";

        return false;
    }

    uint8_t buffer[MAX_MESSAGE_SIZE];

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

    if (!pb_encode(&stream, ModuleMessage_fields, &message))
    {
        std::cerr << "Failed to encode protobuf: " << PB_GET_ERROR(&stream) << '\n';

        return false;
    }

    uint32_t messageSize = static_cast<uint32_t>(stream.bytes_written);

    uint8_t header[4]
    {
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

    // std::cout
    //     << "Sent protobuf message ("
    //     << messageSize << " bytes) on socket " << socket << '\n';

    return true;
}


bool TcpConnection::SendAll(const uint8_t* buffer, size_t size)
{
#ifdef _WIN32

    size_t totalSent = 0;

    while (totalSent < size)
    {
        int bytesSent = send(
            static_cast<SOCKET>(socket),
            reinterpret_cast<const char*>(buffer + totalSent),
            static_cast<int>(size - totalSent),
            0
        );

        if (bytesSent == SOCKET_ERROR)
        {
            int error = WSAGetLastError();

            /*
             * Because this socket is non-blocking, send()
             * may temporarily be unable to accept more data.
             *
             * For our small protobuf messages, simply report
             * the failure rather than blocking the game loop.
             */
            std::cerr << "send() failed on socket " << socket << " with error " << error << '\n';

            return false;
        }

        if (bytesSent == 0)
        {
            return false;
        }

        totalSent += bytesSent;
    }

    return true;

#else

    return false;

#endif
}

void TcpConnection::HandleHeartbeat(const ModuleMessage &message) {
    lastHeartbeat = std::chrono::steady_clock::now();

    // std::cout << "Heartbeat received from " << message.device_id << " on socket " << socket << '\n';
}

bool TcpConnection::CheckHeartbeatTimeout()
{
    const auto now = std::chrono::steady_clock::now();

    const auto elapsed =std::chrono::duration_cast<std::chrono::seconds>(now - lastHeartbeat);

    if (elapsed >= HEARTBEAT_TIMEOUT)
    {
        std::cout << "Heartbeat timeout for socket " << socket << " (" << elapsed.count() << " seconds)\n";

        Close();

        return false;
    }

    return true;
}


void TcpConnection::Close()
{
    if (!connected)
    {
        return;
    }

    std::cout << "Closing TCP connection: " << socket << '\n';

#ifdef _WIN32

    /*
     * Tell the remote endpoint that both directions of the
     * connection are being shut down.
     */
    shutdown(static_cast<SOCKET>(socket), SD_BOTH);

    closesocket(static_cast<SOCKET>(socket));

#endif

    connected = false;
    socket = -1;
    receiveBuffer.clear();
}
