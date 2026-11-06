//
// Created by Weckest on 2026-09-29.
//

#include "ActionManager.h"

#include "ecs/World.h"


ActionManager::ActionManager(World &world) : world(world) {}

void ActionManager::handleAction(const ModuleMessage &message) {
    for (auto& ia : currentInputActionContext->actions) {
        // std::cout << message.device_id << ", " << ia.deviceId << std::endl;
        // std::cout << message.which_payload << ", " << ia.inputType << std::endl;
        // std::cout << message.payload.input_message.inputId << ", " << ia.inputId << std::endl;
        if (message.device_id == ia.deviceId) {
            if (message.which_payload == ModuleMessage_input_message_tag) {
                if (message.payload.input_message.which_input == ia.inputType) {
                    if (message.payload.input_message.inputId == ia.inputId) {
                        world.getEventManager().emit(ActionEvent{new Action{message.device_id, nullptr, ia.actionType}});
                    }
                }
            }
        }
    }
}

void ActionManager::addAction(pb_size_t messageType, const std::string &deviceId, int inputId, std::string actionType) {
    auto context = inputActionContexts.find("base");
    if (context != inputActionContexts.end()) {
        context->second.actions.push_back({messageType, deviceId, inputId, actionType});
    } else {
        inputActionContexts.try_emplace("base", InputActionContext{"base", {{messageType, deviceId, inputId, actionType}}});
    }
}

void ActionManager::addActionContext(InputActionContext inputActioContext) {
    inputActionContexts.try_emplace(inputActioContext.name, inputActioContext);
}

void ActionManager::switchCurrentContext(std::string contextName) {
    auto context = inputActionContexts.find(contextName);
    if (context != inputActionContexts.end()) {
        currentInputActionContext = &context->second;
    } else {
        std::cout << "Context: " << contextName << " missing" << std::endl;
    }
}

InputActionContext * ActionManager::getCurrentContext() {
    return currentInputActionContext;
}
