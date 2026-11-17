//
// Created by Weckest on 2026-11-10.
//

#include "TestSystem.h"

#include "ecs/World.h"

TestSystem::TestSystem(World &world) {
    world.getSystemManager().addOperation("test.print",
        [](ActionContext& context, World& world) {
            if (!context.hasParameter("message")) return -1;
            std::cout << *context.getParameter<std::string>("message") << std::endl;
            return 1;
        }, {"message"});
}
