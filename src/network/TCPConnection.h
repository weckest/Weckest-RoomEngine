//
// Created by Weckest on 2026-09-28.
//

#pragma once

#include <chrono>
#include <cstdint>
#include <vector>
#include <cstddef>

#include "hardware.pb.h"

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <winsock2.h>
    using TcpSocket = SOCKET;
inline constexpr TcpSocket INVALID_TCP_SOCKET = INVALID_SOCKET;
#else
using TcpSocket = int;
inline constexpr TcpSocket INVALID_TCP_SOCKET = -1;
#endif

class TcpConnection
{
public:
    explicit TcpConnection(TcpSocket socket);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    bool IsConnected() const;

    TcpSocket GetSocket() const;

    /*
     * Reads any currently available TCP data.
     *
     * Because TCP does not preserve message boundaries, this method
     * stores partial data internally until a complete framed protobuf
     * message has been received.
     *
     * Returns:
     *   true  - connection is still alive
     *   false - connection has been closed or encountered an error
     */
    bool Update(std::vector<ModuleMessage>& messages);

    bool Send(const ModuleMessage& message);

    void Close();

private:
    bool SetNonBlocking();

    bool ProcessReceiveBuffer(std::vector<ModuleMessage>& messages);

    bool DecodeMessage(
        const uint8_t* buffer,
        size_t size,
        ModuleMessage& message
    );

    bool SendAll(const uint8_t* buffer, size_t size);

    void HandleHeartbeat(const ModuleMessage& message);

    bool CheckHeartbeatTimeout();

    TcpSocket socket;
    bool connected;

    std::vector<uint8_t> receiveBuffer;

    std::chrono::steady_clock::time_point lastHeartbeat;

    static constexpr uint32_t MAX_MESSAGE_SIZE = 1024;

    static constexpr std::chrono::seconds HEARTBEAT_TIMEOUT{6};
};
