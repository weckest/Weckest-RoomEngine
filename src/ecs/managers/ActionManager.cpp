//
// Created by Weckest on 2026-09-29.
//

#include "ActionManager.h"

#include "ecs/World.h"


ActionManager::ActionManager(World &world) : world(world) {}

void ActionManager::handleAction(const ModuleMessage &message) {
    for (auto& ia : currentInputActionContext->actions()) {
        // std::cout << message.device_id << ", " << ia.deviceId << std::endl;
        // std::cout << message.which_payload << ", " << ia.inputType << std::endl;
        // std::cout << message.payload.input_message.inputId << ", " << ia.inputId << std::endl;
        if (message.device_id == ia.deviceid()) {
            if (message.which_payload == ModuleMessage_input_message_tag) {
                if (message.payload.input_message.which_input == ia.inputevent()) {
                    if (message.payload.input_message.inputId == ia.inputid()) {
                        Action action;
                        action.CopyFrom(ia.action());
                        world.getEventManager().emit(ActionEvent{&action});
                    }
                }
            }
        }
    }
}

// void ActionManager::addAction(pb_size_t messageType, const std::string &deviceId, int inputId, std::string actionType, std::string contextName) {
//     auto context = inputActionContexts.find(contextName);
//     if (context != inputActionContexts.end()) {
//         context->second.actions.push_back({messageType, deviceId, inputId, actionType});
//     } else {
//         inputActionContexts.emplace_back(InputActionContext{"base", {{messageType, deviceId, inputId, actionType}}});
//     }
//
//     for (auto inputActionContext : inputActionContexts) {
//         if (inputActionContext.name() == contextName) {
//             auto newAction = inputActionContext.add_actions();
//             newAction.
//         }
//     }
// }

void ActionManager::addActionContext(InputActionContext inputActioContext) {
    inputActionContexts.emplace_back(inputActioContext);
}

void ActionManager::switchCurrentContext(std::string contextName) {
    for (auto& ia : inputActionContexts) {
        if (ia.name() == contextName) {
            currentInputActionContext = &ia;
            return;
        }
    }
    std::cout << "Could not find input context: " << contextName << std::endl;
}

InputActionContext * ActionManager::getCurrentContext() {
    return currentInputActionContext;
}

std::vector<InputActionContext> & ActionManager::getInputActionContexts() {
    return inputActionContexts;
}

Action * ActionManager::getAction(Action *requirement) {
    for (auto ia : inputActionContexts) {
        for (auto action : ia.actions()) {
            if (requirement->name() == action.action().name()) {
                return action.mutable_action();
            }
        }
    }
    return nullptr;
}
