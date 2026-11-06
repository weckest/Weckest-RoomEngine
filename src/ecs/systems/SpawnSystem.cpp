//
// Created by Weckest on 2026-11-02.
//

#include "SpawnSystem.h"

#include "ecs/World.h"

SpawnSystem::SpawnSystem(World &world) {
    // 1. Obtain a random seed from the hardware
    std::random_device rd;

    // 2. Initialize the standard Mersenne Twister engine with the seed
    std::mt19937 gen(rd());

    world.getSystemManager().addOperation("spawn.puzzle.piece",

        [&rd, *this](ActionContext& context, World& world) {
           if (!context.hasParameter("spawn")) return -1;

            Transform transform;
            std::mt19937 gen(rd());
            transform.position.y = generateRandomNumber(gen, -10, 10);
            transform.position.x = generateRandomNumber(gen, -10, 10);

            auto spawn = context.getParameter<std::string>("spawn");
            std::cout << "Spawning: " << *spawn << std::endl;
            world.getRenderService().SpawnEntity(*context.getParameter<std::string>("spawn"), transform);
            return 1;
        });

    world.getSystemManager().addOperation("spawn.entity",
        [](ActionContext& context, World& world) {
            if (!context.hasParameter("entity.tag")) return -1;

            for (auto& entity: world.getEntities()) {
                if (entity->hasComponent<Tag>()) {
                    auto& tag = entity->getComponent<Tag>();

                    Vector3D position = {};

                    if (entity->hasComponent<Transform>()) {
                        position = entity->getComponent<Transform>().position;
                    }

                    if (tag.tag == *context.getParameter<std::string>("entity.tag")) {
                        world.getRenderService().SpawnEntity(tag.tag, {position, 1.0f});
                        tag.rendered = true;
                        return 1;
                    }
                }
            }

            world.getRenderService().SpawnEntity(*context.getParameter<std::string>("entity.tag"), {{}, 1.0f});

            return 1;
        }, {"entity.tag"});
}

int SpawnSystem::generateRandomNumber(std::mt19937 &gen, int min, int max) const {
    std::uniform_int_distribution<> distr(min, max);
    return distr(gen);
}
