//
// Created by Weckest on 2026-11-03.
//

#include "EnemySystem.h"

#include "ecs/World.h"

EnemySystem::EnemySystem(World &world) {
    world.getSystemManager().addOperation("enemy.ai.attack",
        [](ActionContext& context, World& world) {
            if (!context.source || !context.target) return -1;

            if (context.target.value()->hasComponent<Enemy>()&&
                context.target.value()->hasComponent<Animations>() &&
                context.target.value()->hasComponent<Tag>() &&
                context.source.value()->hasComponent<Health>()) {

                auto& enemy = context.target.value()->getComponent<Enemy>();
                auto& animations = context.target.value()->getComponent<Animations>();
                auto& tag = context.target.value()->getComponent<Tag>();
                auto& health = context.source.value()->getComponent<Health>();

                capstone::render::RenderEvent event;

                if (tag.rendered) {
                    event.set_entity_id(tag.tag);
                    event.set_type(capstone::render::RENDER_PLAY_ANIMATION);
                    auto* a = event.mutable_play_animation();
                    a->set_animation_name(animations.animations.find("attack")->second);
                    a->set_animation_after(animations.animations.find("idle")->second);
                    world.getRenderService().SendEvent(event);

                    health.health = std::clamp(health.health - 20.0f, 0.0f, 100.0f);
                    std::cout << "Health: " << health.health << std::endl;
                }
            }

            return 1;
        });
}
