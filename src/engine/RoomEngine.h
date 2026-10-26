//
// Created by Weckest on 2026-09-28.
//

#pragma once
#include "ecs/World.h"
#include "network/TcpServer.h"

class RoomEngine {
public:
    explicit RoomEngine(int port);
    bool Start();
    void Run();
    void Stop();

    bool running() {
        return isRunning;
    }

    World& getWorld() {
        return world;
    }

private:

    TcpServer tcpServer;
    World world;

    const int FPS = 60;
    const float desiredFrameTime = 1000.0f/FPS;

    bool isRunning = false;
};
