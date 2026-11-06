//
// Created by Weckest on 2026-10-26.
//

#include "SceneSystem.h"

#include "ecs/World.h"

SceneSystem::SceneSystem(World &world) {
    world.getSystemManager().addOperation("scene.switch",
        [](ActionContext& context, World& world) {
            if (!context.hasParameter("sceneName")) return -1;

            const auto* sceneName = context.getParameter<std::string>("sceneName");

            if (!sceneName) return -1;

            std::cout << "Switching Scene to " <<  *sceneName << std::endl;

            capstone::render::RenderEvent event;
            event.set_type(capstone::render::RENDER_START_SCENE);

            auto* ss = event.mutable_start_scene();
            ss->set_scene_name(*sceneName);

            world.getRenderService().SendEvent(event);
            return 1;
    }, {"sceneName"});
}
