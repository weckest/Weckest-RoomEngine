//
// Created by Weckest on 2026-09-28.
//
#pragma once

#include <chrono>
#include <cstdint>
#include <vector>

#include "hardware.pb.h"

class TcpConnection
{
public:
    explicit TcpConnection(int socket);
    ~TcpConnection();

    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    bool IsConnected() const;

    int GetSocket() const;

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

    void HandleHeartbeat(
        const ModuleMessage& message
    );

    bool CheckHeartbeatTimeout();

    int socket;
    bool connected;

    std::vector<uint8_t> receiveBuffer;

    std::chrono::steady_clock::time_point lastHeartbeat;

    static constexpr uint32_t MAX_MESSAGE_SIZE = 1024;

    static constexpr std::chrono::seconds HEARTBEAT_TIMEOUT{
        6
    };
};
