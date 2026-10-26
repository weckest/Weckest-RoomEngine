
//
// Created by Weckest on 2026-09-23.
//

#pragma once

#include "hardware.pb.h"
#include "data/Actions.h"
#include "network/IModuleSender.h"

class TcpConnection;

class ModuleService
{
public:
    explicit ModuleService(
        IModuleSender& sender
    );

    void HandleMessage(
        const ModuleMessage& message,
        int clientSocket,
        TcpConnection& connection
    );

    void OnModuleConnected(const ModuleMessage& message, int clientSocket);

    void HandleOutput(OutputAction& outputAction);

private:
    void HandleHeartbeat(
        const ModuleMessage& message,
        int clientSocket,
        TcpConnection& connection
    );

    void SendModuleSetup(const std::string& deviceId, int clientSocket);


    IModuleSender& sender;


};

