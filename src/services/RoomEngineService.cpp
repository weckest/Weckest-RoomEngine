//
// Created by Weckest on 2026-11-09.
//

#include "RoomEngineService.h"

#include "ecs/World.h"

void RoomEngineService::start() {
    const std::string address = "0.0.0.0:5003";

    grpc::ServerBuilder builder;

    builder.AddListeningPort(
        address,
        grpc::InsecureServerCredentials()
    );

    builder.RegisterService(this);

    server_ = builder.BuildAndStart();

    if (!server_)
    {
        throw std::runtime_error("Failed to start RoomEngine gRPC server");
    }

    std::cout << "RoomEngine gRPC server listening on "
              << address << std::endl;

    serverThread_ = std::thread([this]()
    {
        server_->Wait();

        std::cout << "RoomEngine gRPC server stopped." << std::endl;
    });
}

void RoomEngineService::stop() {
    if (server_)
    {
        std::cout << "Stopping RoomEngine gRPC server..." << std::endl;

        server_->Shutdown();

        if (serverThread_.joinable())
        {
            serverThread_.join();
        }

        server_.reset();

    }
}

grpc::Status RoomEngineService::GetSystems(grpc::ServerContext *context, const Empty *request,
                                           GameSystems *response) {

    std::cout << "Getting systems" << std::endl;

    auto& operations = world.getSystemManager().getOperations();

    for (const auto& operation : operations)
    {
        auto* system = response->add_systems();

        system->set_name(operation.second.name);

        for (const std::string& requirement : operation.second.requirements)
        {
            system->add_requirements(requirement);
        }

        for (const std::string& output : operation.second.outputs)
        {
            system->add_outputs(output);
        }
    }

    return grpc::Status::OK;
}

grpc::Status RoomEngineService::GetActions(grpc::ServerContext *context, const Empty *request, Actions *response) {

    std::cout << "Getting actions" << std::endl;

    auto actions = world.getActionManager().getInputActionContexts();
    for (auto context : actions) {
        auto inputContext = response->add_inputcontexts();
        inputContext->set_name(context.name());

        for (auto action : context.actions()) {
            auto a = inputContext->add_actions();
            a->CopyFrom(action);
        }
    }

    return grpc::Status::OK;
}

grpc::Status RoomEngineService::GetRoomEvents(grpc::ServerContext *context, const Empty *request,
                                              RoomEvents *response) {

    std::cout << "Getting room events" << std::endl;

    auto roomEvents = world.getRoomEventManager().getRoomEvents();

    if (roomEvents.empty()) {
        std::cout << "No Room Events" << std::endl;
    }

    for (const auto& roomEvent : roomEvents) {
        auto event = response->add_events();
        event->set_name(roomEvent.name());

        std::cout << event->name() << std::endl;

        for (auto& requirement : roomEvent.requirements()) {
            event->add_requirements()->CopyFrom(requirement);
        }

        for (auto& operation : roomEvent.operations()) {
            event->add_operations()->CopyFrom(operation);
        }

        for (auto& output : roomEvent.outputs()) {
            event->add_outputs()->CopyFrom(output);
        }

        for (auto& param : roomEvent.parameters()) {
            event->add_parameters()->CopyFrom(param);
        }

        event->set_ready(roomEvent.ready());
        event->set_cooldown(roomEvent.cooldown());
        event->set_currentcooldown(roomEvent.currentcooldown());
    }

    return grpc::Status::OK;
}

grpc::Status RoomEngineService::GetRoomEvent(grpc::ServerContext *context, const Name *request,
    RoomEvent *response) {

    auto roomEvent = world.getRoomEventManager().getRoomEvent(request->name());
    if (roomEvent) {
        auto event = response;
        event->set_name(roomEvent->name());

        std::cout << event->name() << std::endl;

        for (auto& requirement : roomEvent->requirements()) {
            event->add_requirements()->CopyFrom(requirement);
        }

        for (auto& operation : roomEvent->operations()) {
            event->add_operations()->CopyFrom(operation);
        }

        for (auto& param : roomEvent->parameters()) {
            event->add_parameters()->CopyFrom(param);
        }

        for (auto& output : roomEvent->outputs()) {
            event->add_outputs()->CopyFrom(output);
        }

        event->set_ready(roomEvent->ready());
        event->set_cooldown(roomEvent->cooldown());
        event->set_currentcooldown(roomEvent->currentcooldown());

        return grpc::Status::OK;
    }

    return grpc::Status::CANCELLED;
}

grpc::Status RoomEngineService::UpdateRoomEvent(grpc::ServerContext *context, const RoomEvent *request,
                                                Empty *response) {

    std::cout << "Updating room event: " << request->name() << std::endl;

    auto roomEvent = world.getRoomEventManager().getRoomEvent(request->name());

    if (roomEvent) {
        std::cout << "Updating Room Event: " << request->name() << std::endl;
        roomEvent->CopyFrom(*request);

    } else {
        std::cout << "Adding new Room Event: " << request->name() << std::endl;
        //make a new room event
        auto roomEvent = world.getRoomEventManager().addRoomEvent(request->name());
        roomEvent->CopyFrom(*request);
    }

    return grpc::Status::OK;
}

grpc::Status RoomEngineService::RemoveRoomEvent(grpc::ServerContext *context, const Name *request, Empty *response) {

    std::cout << "Removing room event: " << request->name() << std::endl;

    auto roomEvent = world.getRoomEventManager().getRoomEvent(request->name());

    if (roomEvent) {
        auto front = world.getRoomEventManager().getRoomEvents().begin();
        while (front->id() != roomEvent->id()) {
            front.operator++();
        }
        if (front != world.getRoomEventManager().getRoomEvents().end()) {
            world.getRoomEventManager().getRoomEvents().erase(front);
        }
    }

    return grpc::Status::OK;
}
