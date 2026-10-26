//
// Created by Weckest on 2026-09-23.
//

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "IModuleSender.h"
#include "TcpConnection.h"
#include "services/ModuleService.h"

class TcpServer : public IModuleSender
{
public:
    explicit TcpServer(int port);
    ~TcpServer();

    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    bool Start();

    /*
     * Processes networking for all connected modules.
     *
     * This should be called repeatedly by RoomEngine.
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
    bool Send(int clientSocket, const ModuleMessage& message) override;

private:
    bool SetupServerSocket();

    void AcceptNewConnections();

    void ProcessConnections();

    void RemoveDisconnectedConnections();

    int port;
    int serverSocket;

    bool running;

    std::vector<std::unique_ptr<TcpConnection>> connections;

    ModuleService moduleService;
};
