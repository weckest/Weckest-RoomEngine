//
// Created by Weckest on 2026-09-28.
//

#pragma once
#include <string>

#include "Data.h"
#include "network/TCPConnection.h"

struct Module {
    std::string deviceId{};
    TcpConnection* tcpConnection;
    int time{};
};

struct Hardware {
    std::vector<Light> lights;
};
