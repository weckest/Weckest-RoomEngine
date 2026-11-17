//
// Created by Weckest on 2026-09-29.
//

#ifndef ROOMENGINE_ACTIONMANAGER_H
#define ROOMENGINE_ACTIONMANAGER_H
#include <unordered_map>
#include <vector>

#include "hardware.pb.h"
#include "utils/ProtoAlias.h"

class World;

class ActionManager {
    std::vector<InputActionContext> inputActionContexts;
    InputActionContext* currentInputActionContext;
    World& world;

public:
    ActionManager(World& world);
    void handleAction(const ModuleMessage& message);

    // void addAction(pb_size_t messageType, const std::string &deviceId, int inputId, std::string actionType, std::string contextName = "base");

    void addActionContext(InputActionContext inputActioContext);

    void switchCurrentContext(std::string contextName);

    InputActionContext* getCurrentContext();

    std::vector<InputActionContext>& getInputActionContexts();

    Action* getAction(Action* requirement);
};


#endif //ROOMENGINE_ACTIONMANAGER_H