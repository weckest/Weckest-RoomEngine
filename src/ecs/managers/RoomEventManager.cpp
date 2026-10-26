//
// Created by Weckest on 2026-09-29.
//

#include "RoomEventManager.h"

#include <any>

#include "ecs/World.h"



void RoomEventManager::addRoomEvent(
    std::string name,
    std::vector<ActionType> requirements,
    std::vector<std::string> operations,
    std::vector<OutputAction> outputActions,
    float cooldown) {
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

    roomEvents.insert({name, re});

    std::cout << name << " Room Event Added" << std::endl;

}
