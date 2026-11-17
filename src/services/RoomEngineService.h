//
// Created by Weckest on 2026-11-09.
//

#ifndef ROOMENGINE_ROOMENGINESERVICE_H
#define ROOMENGINE_ROOMENGINESERVICE_H

#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>
#include "roomengine.grpc.pb.h"

class World;

using GameSystems = weckest::game::GameSystems;
using Name = weckest::game::Name;
using System = weckest::game::System;
using Empty = weckest::game::Empty;
using RoomEvents = weckest::game::RoomEvents;
using RoomEvent = weckest::game::RoomEvent;
using Actions = weckest::game::Actions;


class RoomEngineService final : public weckest::game::RoomEngineService::Service {
    World& world;

private:
    std::unique_ptr<grpc::Server> server_;
    std::thread serverThread_;

public:
    explicit RoomEngineService(World& world) : world(world) {};
    void start();
    void stop();

    grpc::Status GetSystems(
        grpc::ServerContext* context,
        const Empty* request,
        GameSystems* response
    ) override;

    grpc::Status GetActions(
        grpc::ServerContext *context,
        const Empty *request,
        Actions *response) override;

    grpc::Status GetRoomEvents(
        grpc::ServerContext *context,
        const Empty *request,
        RoomEvents *response) override;

    grpc::Status GetRoomEvent(
        grpc::ServerContext *context,
        const Name *request,
        RoomEvent *response) override;

    grpc::Status UpdateRoomEvent(
        grpc::ServerContext *context,
        const RoomEvent *request,
        Empty *response) override;

    grpc::Status RemoveRoomEvent(
        grpc::ServerContext *context,
        const Name *request,
        Empty *response) override;

};


#endif //ROOMENGINE_ROOMENGINESERVICE_H