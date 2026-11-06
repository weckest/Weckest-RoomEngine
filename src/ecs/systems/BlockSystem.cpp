//
// Created by Weckest on 2026-11-03.
//

#include "BlockSystem.h"

#include "ecs/World.h"

BlockSystem::BlockSystem(World &world) {
    world.getSystemManager().addOperation("block",
        [](ActionContext& context, World& world) {

            return 1;
        });
}
