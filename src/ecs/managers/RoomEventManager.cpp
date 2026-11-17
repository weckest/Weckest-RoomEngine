//
// Created by Weckest on 2026-09-29.
//

#include "RoomEventManager.h"

#include <any>

#include "ecs/World.h"

void RoomEventManager::addRoomEvent(std::string name, std::vector<Action> requirements,
    std::vector<System> operations, std::vector<Output> outputActions, float cooldown,
    std::vector<Parameter> parameters) {
    std::cout << "Adding Room Event: " << name << std::endl;
    RoomEvent re = RoomEvent();

    re.set_name(name);

    for (auto requirement : requirements) {
        re.add_requirements()->CopyFrom(requirement);
    }

    SystemManager& systemManager = world.getSystemManager();

    bool flag = true;
    for (auto operation : operations) {
        if (!systemManager.getOperation(operation.name())) {
            std::cout << "Operation \"" << operation.name() << "\" not found for Room Event: " << name << std::endl;
            flag = false;
        }
    }

    if (flag) {
        for (auto operation : operations) {
            re.add_operations()->CopyFrom(operation);
        }
    }

    for (auto output : outputActions) {
        re.add_outputs()->CopyFrom(output);
    }

    re.set_cooldown(cooldown);

    for (auto parameter : parameters) {
        re.add_parameters()->CopyFrom(parameter);
    }

    roomEvents.emplace_back(re);

    // std::cout << name << " Room Event Added" << std::endl;
}

RoomEvent * RoomEventManager::addRoomEvent(std::string name) {
    RoomEvent re;
    re.set_name(name);
    roomEvents.emplace_back(re);
    return getRoomEvent(name);
}

void RoomEventManager::addRoomEvent(RoomEvent *roomEvent) {
    std::cout << "Adding Room Event: " << roomEvent->name() << std::endl;
    roomEvents.emplace_back(*roomEvent);
}


RoomEvent * RoomEventManager::getRoomEvent(std::string name) {
    for (auto& re : roomEvents) {
        if (re.name() == name) {
            return &re;
        }
    }

    std::cerr << "Room Event \"" << name << "\" not found" << std::endl;

    // Create a safe, permanent "null" Operation to return
    return nullptr;
}

void RoomEventManager::triggerRoomEvent(std::string name) {
    RoomEvent roomEvent;
    for (auto& re : roomEvents) {
        if (re.name() == name) {
            roomEvent.CopyFrom(re);
        }
    }
    if (roomEvent.name() == name) {
        std::cout << "Triggering: " << roomEvent.name() << std::endl;
        for (auto requirement : roomEvent.requirements()) {
            std::cout << " Adding requirement: " << requirement.name() << std::endl;
            roomEvent.add_actions()->CopyFrom(*world.getActionManager().getAction(&requirement));
        }
    }

}
