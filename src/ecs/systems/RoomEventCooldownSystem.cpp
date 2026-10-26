//
// Created by Weckest on 2026-10-20.
//

#include "RoomEventCooldownSystem.h"

#include "ecs/World.h"

void RoomEventCooldownSystem::update(World &world) {
    auto& events = world.getRoomEventManager().getRoomEvents();

    for (auto& event : events) {
        if (!event.second.ready && event.second.currentCooldown <= 0) {
            event.second.ready = true;

            auto& module = world.getPlayer()->getComponent<Module>();

            ModuleMessage message = ModuleMessage_init_zero;

            strcpy(message.device_id, module.deviceId.c_str());
            message.which_payload = ModuleMessage_set_led_tag;
            message.payload.set_led.led_id = 1;
            message.payload.set_led.state = event.second.ready;

            module.tcpConnection->Send(message);
        } else {
            event.second.currentCooldown--;
        }
    }
}
