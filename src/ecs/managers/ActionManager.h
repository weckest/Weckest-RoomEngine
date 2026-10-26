//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ACTIONMANAGER_H
#define ROOMENGINE_ACTIONMANAGER_H
#include <vector>

#include "hardware.pb.h"
#include "data/Actions.h"


class World;

class ActionManager {
    std::vector<InputAction> inputActions;
    World& world;

public:
    ActionManager(World& world);
    void handleAction(const ModuleMessage& message);

    void addAction(pb_size_t messageType, const std::string &deviceId, int inputId, ActionType actionType);
};


#endif //ROOMENGINE_ACTIONMANAGER_H