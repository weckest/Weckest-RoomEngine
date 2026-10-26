//
// Created by Weckest on 2026-09-29.
//

#include "ActionManager.h"

#include "ecs/World.h"


ActionManager::ActionManager(World &world) : world(world) {}

void ActionManager::handleAction(const ModuleMessage &message) {
    for (auto& ia : inputActions) {
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

void ActionManager::addAction(pb_size_t messageType, const std::string &deviceId, int inputId, ActionType actionType) {
    std::cout << "Adding action " << deviceId << " " << inputId << std::endl;
    inputActions.push_back({messageType, deviceId, inputId, actionType});
}
