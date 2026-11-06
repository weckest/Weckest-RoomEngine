//
// Created by Weckest on 2026-11-03.
//

#include "ContextSystem.h"

#include "ecs/World.h"

ContextSystem::ContextSystem(World &world) {
    world.getSystemManager().addOperation("context.switch",
        [](ActionContext& context, World& world) {
            if (!context.hasParameter("context")) return -1;

            auto inputContext = context.getParameter<std::string>("context");

            world.getActionManager().switchCurrentContext(*inputContext);

            return 1;
    });
}
