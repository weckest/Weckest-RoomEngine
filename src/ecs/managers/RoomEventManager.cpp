//
// Created by Weckest on 2026-09-29.
//

#include "RoomEventManager.h"

#include <any>

#include "ecs/World.h"



void RoomEventManager::addRoomEvent(
    std::string name,
    std::vector<std::string> requirements,
    std::vector<std::string> operations,
    std::vector<std::string> outputActions,
    float cooldown,
    std::unordered_map<std::string, std::string> parameters
) {
    std::cout << "Adding Room Event: " << name << std::endl;
    RoomEvent re = RoomEvent();

    re.name = name;
    re.actions = std::vector<Action>();

    re.requirements = requirements;

    SystemManager& systemManager = world.getSystemManager();

    bool flag = true;
    for (std::string operation : operations) {
        if (!systemManager.getOperation(operation)) {
            std::cout << "Operation \"" << operation << "\" not found for Room Event: " << name << std::endl;
            flag = false;
        }
    }

    if (flag) re.operations = operations;

    re.outputActions = outputActions;

    re.cooldown = cooldown;

    re.parameters = parameters;

    roomEvents.insert({name, re});

    // std::cout << name << " Room Event Added" << std::endl;

}

void RoomEventManager::triggerRoomEvent(std::string name) {
    auto roomEvent = roomEvents.find(name);
    if (roomEvent != roomEvents.end()) {
        std::cout << "Triggering: " << roomEvent->second.name << std::endl;
        for (std::string requirement : roomEvent->second.requirements) {
            std::cout << " Adding requirement: " << requirement << std::endl;
            roomEvent->second.actions.emplace_back(Action{"", nullptr, requirement});
        }
    }

}
