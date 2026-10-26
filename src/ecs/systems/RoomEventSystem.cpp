//
// Created by Weckest on 2026-09-29.
//

#include "RoomEventSystem.h"

#include "data/ActionContext.h"
#include "data/Operation.h"
#include "ecs/World.h"

RoomEventSystem::RoomEventSystem(World &world) {
    world.getEventManager().subscribe(
        [this, &world](const BaseEvent& e) {
            if (e.type != EventType::InputAction) return;

            const auto& input = static_cast<const ActionEvent&>(e);

            for (std::pair<const std::string, RoomEvent>& kvp : world.getRoomEventManager().getRoomEvents()) {
                std::cout << "Adding action: " << input.action->actionType << std::endl;
                if (kvp.second.ready) {
                    kvp.second.actions.push_back(*input.action);
                }
            }
    });
}

void RoomEventSystem::update(World &world, float dt) {
    for (std::pair<const std::string, RoomEvent>& kvp : world.getRoomEventManager().getRoomEvents()) {
        RoomEvent& roomEvent = kvp.second;

        std::vector<bool> flags;

        for (ActionType& requirement : roomEvent.requirements) {
            bool found = false;
            for (Action& action : roomEvent.actions) {
                if (action.actionType == requirement) {
                    found = true;
                    flags.push_back(true);
                }
            }
            if (!found) {
                flags.push_back(false);
            }
        }

        bool foundAll = true;
        for (bool flag : flags) {
            if (!flag) {
                foundAll = false;
            }
        }

        if (foundAll) {
            std::cout << "Requirements met" << std::endl;

            //start Room Event cooldown
            roomEvent.ready = false;
            roomEvent.currentCooldown = roomEvent.cooldown;

            auto& module = world.getPlayer()->getComponent<Module>();

            ModuleMessage message = ModuleMessage_init_zero;

            strcpy(message.device_id, module.deviceId.c_str());
            message.which_payload = ModuleMessage_set_led_tag;
            message.payload.set_led.led_id = 1;
            message.payload.set_led.state = roomEvent.ready;

            module.tcpConnection->Send(message);

            //shared action context
            ActionContext context{};
            context.source = world.getPlayer();
            context.target = world.getPlayer();

            SystemManager& systemManager = world.getSystemManager();
            for (std::string operation : roomEvent.operations) {
                //check that the context has the requirements for the operation
                auto& o = *systemManager.getOperation(operation);
                bool contextGood = true;

                for (std::string requirement : o.requirements) {
                    if (!context.hasParameter(requirement)) {
                        contextGood = false;
                        std::cout << "Error: Operation " << operation << " missing " << requirement << std::endl;

                        auto ops = world.getSystemManager().getOperationsThatOutput(requirement);

                        if (!ops.empty()) {
                            std::cout << "The following operations outputs the needed parameters: " << std::endl;
                            for (std::string op : ops) {
                                std::cout << " - " <<  op << std::endl;
                            }
                        }
                        break;
                    }
                }

                if (contextGood) {
                    int success = systemManager.getOperation(operation)->function(context, world);
                    if (success < 0) {
                        std::cout << "Error: Operation Failed: " << operation << " " << success << std::endl;
                        break;
                    }
                }
            }
        }

        roomEvent.actions.clear();
    }
}
