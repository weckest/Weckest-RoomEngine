//
// Created by Weckest on 2026-09-28.
//

#include "RoomEngine.h"

#include <thread>


RoomEngine::RoomEngine(int port) : tcpServer(port), world(tcpServer) {}

bool RoomEngine::Start() {
    if (!tcpServer.Start()) return false;

    isRunning = true;
    return true;
}

void RoomEngine::Run() {
    using Clock = std::chrono::steady_clock;

    auto previousTime = Clock::now();

    while (isRunning && tcpServer.IsRunning())
    {
        auto currentTime = Clock::now();

        const float dt = std::chrono::duration<float>(currentTime - previousTime).count();

        previousTime = currentTime;

        (void)dt;

        tcpServer.Update();
        world.cleanup();
        world.Update(dt);

        currentTime = Clock::now();

        float actualFrameTime = std::chrono::duration<float>(currentTime - previousTime).count();

        if (desiredFrameTime > actualFrameTime) {
            std::this_thread::sleep_for(std::chrono::milliseconds((int)desiredFrameTime - (int)actualFrameTime));
        }
    }
}

void RoomEngine::Stop() {
    isRunning = false;
    tcpServer.Stop();
}
