//
// Created by Weckest on 2026-10-05.
//

#include "RenderServiceClient.h"

#include "render.pb.h"
#include "ecs/components/Component.h"

using RenderEvent = capstone::render::RenderEvent;
using RenderEventType = capstone::render::RenderEventType;
using RenderReply = capstone::render::RenderReply;
using TransformMoved = capstone::render::TransformMoved;
using PlayAnimation = capstone::render::PlayAnimation;
using SpawnEntity = capstone::render::SpawnEntity;

RenderServiceClient::RenderServiceClient(std::shared_ptr<grpc::ChannelInterface> channel) : stub_(capstone::render::RenderService::NewStub(channel)) {}

RenderServiceClient::~RenderServiceClient()
{
    Disconnect();
}

bool RenderServiceClient::Connect()
{
    context_ = std::make_unique<grpc::ClientContext>();

    stream_ = stub_->StreamEvent(context_.get());

    if (!stream_)
    {
        std::cout << "Failed to create RenderEvent stream."
                  << std::endl;

        return false;
    }

    connected_ = true;

    std::cout << "Connected to Godot RenderServer."
              << std::endl;

    readerThread_ = std::thread([this]()
    {
        RenderReply reply;

        while (stream_->Read(&reply))
        {
            std::cout
                << "Godot reply: "
                << reply.entity_id()
                << " - "
                << reply.message()
                << std::endl;
        }

        std::cout
            << "Godot RenderServer disconnected."
            << std::endl;
    });

    return true;
}

void RenderServiceClient::Disconnect()
{
    if (!connected_)
    {
        return;
    }

    std::cout
        << "Disconnecting from Godot RenderServer..."
        << std::endl;

    connected_ = false;

    stream_->WritesDone();

    if (readerThread_.joinable())
    {
        readerThread_.join();
    }

    grpc::Status status = stream_->Finish();

    if (!status.ok())
    {
        std::cout
            << "Render stream ended: "
            << status.error_message()
            << std::endl;
    }

    stream_.reset();
    context_.reset();

    std::cout
        << "Disconnected from Godot."
        << std::endl;
}

bool RenderServiceClient::SendEvent(
    const RenderEvent& event)
{
    if (!connected_)
    {
        std::cout
            << "Cannot send render event: "
               "not connected."
            << std::endl;

        return false;
    }

    return stream_->Write(event);
}

void RenderServiceClient::SpawnEntity(const std::string &entityId, Transform transform)
{
    RenderEvent event;

    event.set_type(
        capstone::render::RENDER_SPAWN_ENTITY);

    event.set_entity_id(entityId);

    auto* spawn = event.mutable_spawn_entity();

    spawn->set_entity_type(entityId);
    spawn->set_x(transform.position.x);
    spawn->set_y(transform.position.y);
    spawn->set_z(transform.position.z);

    std::cout
        << "Sending spawn event for "
        << entityId
        << std::endl;

    SendEvent(event);
}

void RenderServiceClient::EntityMoved(const std::string &entityId, Transform transform) {
    RenderEvent event;

    event.set_type(capstone::render::RENDER_TRANSFORM_MOVED);

    event.set_entity_id(entityId);

    auto* t = event.mutable_transform_moved();

    t->set_x(transform.position.x);
    t->set_y(transform.position.y);
    t->set_z(transform.position.z);

    SendEvent(event);
}

void RenderServiceClient::ChangeColor(const std::string &entityId, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    RenderEvent event;

    event.set_type(capstone::render::RENDER_CHANGE_COLOR);

    event.set_entity_id(entityId);

    auto* c = event.mutable_change_color();
    c->set_r(r);
    c->set_g(g);
    c->set_b(b);
    c->set_a(a);

    SendEvent(event);
}



