//
// Created by Weckest on 2026-10-19.
//

#include "WaitSystem.h"

#include "ecs/World.h"

WaitSystem::WaitSystem(World &world) {
    world.getSystemManager().addOperation("wait",
        [](ActionContext& context, World& world) {

            return 1;
        });
}
