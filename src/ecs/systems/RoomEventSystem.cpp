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

            for (auto roomEvent : world.getRoomEventManager().getRoomEvents()) {
                if (roomEvent.ready()) {
                    // std::cout << "Adding action: " << input.action->actionType << std::endl;
                    roomEvent.add_actions()->CopyFrom(*input.action);
                }
            }
    });
}

void RoomEventSystem::update(World &world, float dt) {
    for (auto& roomEvent : world.getRoomEventManager().getRoomEvents()) {

        if (!roomEvent.ready()) continue;

        std::vector<bool> flags;

        for (auto& requirement : roomEvent.requirements()) {
            bool found = false;
            for (auto action : roomEvent.actions()) {
                if (action.name() == requirement.name()) {
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
            std::cout << "Requirements met for Room Event: " << roomEvent.name() << std::endl;

            //start Room Event cooldown
            roomEvent.set_ready(false);
            roomEvent.set_currentcooldown(roomEvent.cooldown());

            //turn the light off (can be removed in the future)
            //should in theory be a system that can do this
            // auto& module = world.getPlayer()->getComponent<Module>();
            //
            // ModuleMessage message = ModuleMessage_init_zero;
            //
            // strcpy(message.device_id, module.deviceId.c_str());
            // message.which_payload = ModuleMessage_set_led_tag;
            // message.payload.set_led.led_id = 1;
            // message.payload.set_led.state = roomEvent.ready;
            //
            // module.tcpConnection->Send(message);

            //shared action context
            ActionContext context{};
            for (auto& entity : world.getEntities()) {
                if (entity->hasComponent<Enemy>()) {
                    context.target = &*entity;
                    break;
                }
            }
            context.source = world.getPlayer();

            for (auto& parameter : roomEvent.parameters()) {
                context.setParameter(parameter.name(), parameter.value());
            }

            SystemManager& systemManager = world.getSystemManager();
            for (auto& operation : roomEvent.operations()) {
                //check that the context has the requirements for the operation
                auto& o = *systemManager.getOperation(operation.name());
                bool contextGood = true;

                for (std::string requirement : o.requirements) {
                    // std::cout << requirement << std::endl;
                    if (!context.hasParameter(requirement)) {
                        std::cout << "Missing requirement: " << requirement << std::endl;
                        contextGood = false;
                        std::cout << "Error: Operation " << operation.name() << " missing " << requirement << std::endl;

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
                    int success = systemManager.getOperation(operation.name())->function(context, world);
                    if (success < 0) {
                        std::cout << "Error: Operation Failed: " << operation.name() << " " << success << std::endl;
                        break;
                    }
                }
            }

            for (auto& output : roomEvent.outputs()) {
                //THIS SHOULD BE UPDATED IN THE FUTURE TO USE AN ACTUAL DEVICE ID OR HAVE A SEPARATE WAY OF ADDING THESE EVENTS
                Action action;
                action.set_name(output.value());
                world.getEventManager().emit(ActionEvent{&action});
            }
        }

        roomEvent.clear_actions();
    }
}
