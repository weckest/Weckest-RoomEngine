//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ACTIONMANAGER_H
#define ROOMENGINE_ACTIONMANAGER_H
#include <unordered_map>
#include <vector>

#include "hardware.pb.h"
#include "data/Actions.h"
#include "data/InputActionContext.h"


class World;

class ActionManager {
    std::unordered_map<std::string, InputActionContext> inputActionContexts;
    InputActionContext* currentInputActionContext;
    World& world;

public:
    ActionManager(World& world);
    void handleAction(const ModuleMessage& message);

    void addAction(pb_size_t messageType, const std::string &deviceId, int inputId, std::string actionType);
    void addActionContext(InputActionContext inputActioContext);

    void switchCurrentContext(std::string contextName);

    InputActionContext* getCurrentContext();
};


#endif //ROOMENGINE_ACTIONMANAGER_H