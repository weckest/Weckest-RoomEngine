//
// Created by Weckest on 2026-09-23.
//


#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "IModuleSender.h"
#include "TCPConnection.h"
#include "services/ModuleService.h"

class TcpServer : public IModuleSender
{
public:
    explicit TcpServer(int port);
    ~TcpServer() override;

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    bool Start();

    /*
     * Processes networking for all connected modules.
     * Call repeatedly from RoomEngine.
     */
    void Update();

    /*
     * Gracefully closes the server and all module connections.
     */
    void Stop();

    bool IsRunning() const;

    /*
     * Sends a protobuf message to a specific TCP connection.
     */
    bool Send(
        TcpSocket clientSocket,
        const ModuleMessage& message
    ) override;

private:
    bool SetupServerSocket();

    void AcceptNewConnections();
    void ProcessConnections();
    void RemoveDisconnectedConnections();

    int port;
    TcpSocket serverSocket;
    bool running;

#ifdef _WIN32
    bool winsockInitialized = false;
#endif

    std::vector<std::unique_ptr<TcpConnection>> connections;

    ModuleService moduleService;
};

