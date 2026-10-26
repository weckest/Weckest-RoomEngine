//
// Created by Weckest on 2026-09-23.
//

#pragma once

#include "TCPConnection.h"
#include "hardware.pb.h"

class IModuleSender
{
public:
    virtual ~IModuleSender() = default;

    virtual bool Send(
        TcpSocket clientSocket,
        const ModuleMessage& message
    ) = 0;
};