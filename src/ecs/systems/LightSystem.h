//
// Created by Weckest on 2026-09-28.
//

#ifndef ROOMENGINE_LIGHTSYSTEM_H
#define ROOMENGINE_LIGHTSYSTEM_H
#include <iostream>
#include <vector>

#include "hardware.pb.h"
#include "ecs/Entity.h"
#include "ecs/components/Module.h"


class LightSystem {
public:
    void update(std::vector<std::unique_ptr<Entity>>& entities) {
        for (auto& e : entities) {
            if (e->hasComponent<Module>() && e->hasComponent<Hardware>()) {
                auto& module = e->getComponent<Module>();
                auto& hardware = e->getComponent<Hardware>();

                module.time++;

                if (module.time >= 200) {
                    module.time = 0;
                    bool state = hardware.lights.front().state;
                    hardware.lights.front().state = !state;

                    ModuleMessage message = ModuleMessage_init_zero;

                    strcpy(message.device_id, module.deviceId.c_str());
                    message.which_payload = ModuleMessage_set_led_tag;
                    message.payload.set_led.led_id = 2;
                    message.payload.set_led.state = !state;

                    module.tcpConnection->Send(message);
                }

            }
        }
    }
};


#endif //ROOMENGINE_LIGHTSYSTEM_H