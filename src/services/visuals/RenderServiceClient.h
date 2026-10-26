//
// Created by Weckest on 2026-10-05.
//

#pragma once

#include <grpcpp/grpcpp.h>
#include <memory>
#include <thread>
#include <string>

#include "render.grpc.pb.h"
#include "ecs/components/Component.h"


class RenderServiceClient
{
public:
    explicit RenderServiceClient(
        std::shared_ptr<grpc::ChannelInterface> channel);

    ~RenderServiceClient();

    bool Connect();

    void Disconnect();

    bool SendEvent(
        const capstone::render::RenderEvent& event);

    void SpawnEntity(const std::string& entityId, Transform transform);

    void EntityMoved(const std::string& entityId, Transform transform);

    void ChangeColor(const std::string& entityId, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

private:
    std::unique_ptr<capstone::render::RenderService::Stub> stub_;

    std::unique_ptr<grpc::ClientContext> context_;

    std::unique_ptr<
        grpc::ClientReaderWriter<
            capstone::render::RenderEvent,
            capstone::render::RenderReply
        >
    > stream_;

    std::thread readerThread_;

    bool connected_ = false;
};
